# SPDX-License-Identifier: GPL-3.0

# Scanner for XML input files from Juris service.

module Scanner
  class JurisDE
    include PhraseMaskerDE

    PARAGRAPH_RANGED_RE = /\A(?:\(XXXX\)\s+)?§§\s+(\d+)\s+bis\s+(\d+)\z/
    PARAGRAPH_RE = /\A§\s+([0-9a-z]+)\z/
    ARTIKEL_RANGED_RE = /\A(?:\(XXXX\)\s+)?Art\s+(\d+)\s+bis\s+(\d+)\z/
    ARTIKEL_RE = /\AArt\s+([0-9a-z]+)\z/
    ABSATZ_RE = /\A„?\(([0-9][0-9a-z]*)\)/
    ABSATZ_RE_RESCUE = /\A[\(„]?\w+/
    ABSATZ_RE_QUOT = /\A„\(([0-9][0-9a-z]*)\)/
    ABSATZ_RE_QUOT_END = /“(?:<\/[^>]+>)*<\/P>\z/
    APPENDIX_RE = /Anlage\s+([0-9][a-z]*)/

    APPENDIX_ID_LIKE = ["Lfd. Nr."]

    # Names of section found among all types of codes. Preserve the hierarchy!
    SECTION_HIERARCHY = %w(
      Superior
      Buch
      Abschnitt
      Unterabschnitt
      Teil
      Hauptstück
      Titel
      Untertitel
      Kapitel
      Unterkapitel
      Art
      Unterartikel
      Letter
      Roman
      Arabic
      Abteilung
    )

    #--------------------------------------------------------------------------
    def initialize(file_path)
      self.file_path = file_path
    end

    #--------------------------------------------------------------------------
    def action(domain)
      self.domain = domain
      self.abs_pos = 0
      self.stack = []
      open_xml
      scan_code
      parse_norms

      # clear remaining sections
      while stack.length > 1
        stack.pop
      end
      persist_depth
      self.stack = []

      post_processing
    end

    private

    attr_accessor :file_path, :domain, :xml, :stack, :code, :abs_pos

    #--------------------------------------------------------------------------
    def next_pos
      pos_now = abs_pos
      self.abs_pos = abs_pos + 1
      pos_now
    end

    #--------------------------------------------------------------------------
    def open_xml
      File.open(file_path) do |f|
        self.xml = Nokogiri::XML::Document.parse(f, nil, nil, Nokogiri::XML::ParseOptions::STRICT)
      end
    end

    #--------------------------------------------------------------------------
    def persist_depth
      if stack.last[:obj]
        stack.last[:obj].update_attribute(:tree_depth, stack.last[:depth])
      end
    end

    #--------------------------------------------------------------------------
    def shift_back_depth
      bottom = stack.last[:depth]
      for_toc = stack.last[:obj] && [CodeAtom::TYPE_PARAGRAPH, CodeAtom::TYPE_ARTIKEL, CodeAtom::TYPE_ANHANG].include?(stack.last[:obj].atom_type)
      # from TOC, a § is the lowest
      if for_toc
        bottom = 0
      end

      if stack[-2]
        bottom_but_one = stack[-2][:depth]

        if bottom_but_one
          if bottom >= bottom_but_one
            i = stack.size - 2
            j = 1

            until i < 0
              # don't propagate sub-§ things into TOC values
              if !for_toc && stack[i][:obj].atom_type == CodeAtom::TYPE_ABSCHNITT
                break
              end

              stack[i][:depth] = [bottom + j, stack[i][:depth]].max
              i -= 1
              j += 1
            end
          end
        end
      end
    end

    # scan_code creates the code object for this XML file
    #--------------------------------------------------------------------------
    def scan_code
      # Meta data of code, such as names and short names
      real_name = code_name(xml)
      short_amt = xml.xpath("//norm[1]/metadaten/amtabk/text()")[0]
      short_jur = xml.xpath("//norm[1]/metadaten/jurabk/text()")[0]
      short = short_amt || short_jur || real_name

      date = Date.strptime(xml.root["builddate"], "%Y%m%d")

      if real_name
        self.code = Code.create({
          :name => real_name,
          :domain_id => domain.id,
          :short => short.text,
          :version => date
        })

        # Tree root
        master = code.code_atoms.create({
          :position => 0,
          :abs_position => next_pos,
          :key => "---",
          :code_id => code.id,
          :atom_type => CodeAtom::TYPE_MASTER,
          :depth => 0
        })

        stack.push({ :obj => master, :pos => 0, :depth => 0 })

        # Full name as alias
        code.code_aliases.create({ :alias => real_name })
      else
        raise "No real name found!"
      end

      scan_aliases(xml.xpath("//norm[1]")[0])
    end

    #--------------------------------------------------------------------------
    def code_name(xml)
      names = []
      xml.xpath("//norm[1]/metadaten/langue/text()").each { |t| names << t.text.strip }
      name = names.join(" ")
    end

    # Find all kinds of known abbreviations of this code and create an alias
    # for each.
    #--------------------------------------------------------------------------
    def scan_aliases(norm_node)
      jurabks = norm_node.xpath("./metadaten/jurabk")
      amtabks = norm_node.xpath("./metadaten/amtabk")
      kurzues = norm_node.xpath("./metadaten/kurzue")
      abk_nodes = jurabks + amtabks + kurzues

      abks = abk_nodes.map do |abk|
        abk.xpath("./text()")[0].text
      end

      abks.uniq.each do |abk|
        code.code_aliases.create({
          :alias => abk
        })
      end
    end

    # TOC, sections and paragraphs are on equal depth, so we can iterate over
    # them, checking each type.
    #--------------------------------------------------------------------------
    def parse_norms
      xml.xpath("//norm[position()>1]").each do |norm|
        # Check for TOC
        if norm_is_toc?(norm)
          next
        end

        # Section
        if scan_for_section(norm)
          next
        end

        if scan_for_appendix(norm)
          next
        end

        # Paragraph
        if scan_for_paragraph(norm)
          post_paragraph_work(norm)
        end
      end
    end

    #--------------------------------------------------------------------------
    def post_paragraph_work(norm)
      footnotes = norm.xpath("./textdaten/fussnoten/Content")[0]
      if footnotes
        i = 1
        footnotes.xpath("./P").each do |fn|
          extract_footnotes(fn, i)
          i += 1
        end
      end

      shift_back_depth
      persist_depth
      stack.pop
    end

    #--------------------------------------------------------------------------
    def norm_is_toc?(norm)
      text = norm.xpath("./enbez/text()")[0]

      if text
        return text.text == "Inhaltsübersicht"
      end

      false
    end

    # Here, we scan for sections
    #--------------------------------------------------------------------------
    def scan_for_section(norm)
      gb = norm.xpath(".//gliederungsbez/text()")[0]
      enbez = norm.xpath(".//enbez/text()")[0]

      # Some crappy format used in some Verordnungen
      if gb && enbez && gb != enbez
        scan_section_text_combo(norm)
        return true
      end

      if gb
        # Get hierarchy level from name
        sec_depth = classify_section(gb.text)

        if sec_depth.nil?
          raise "UNKNOWN SECTION DEPTH #{gb.text}"
        end

        # This means there is a new section following another one. We need to
        # check its depth and need to pop the stack by the height difference.
        if stack.length > 1 && stack.last[:obj].atom_type == CodeAtom::TYPE_ABSCHNITT
          last_depth = stack.last[:sec_depth]

          if sec_depth <= last_depth
            pops = stack_aware_sec_diff(last_depth, sec_depth, stack) + 1
            shift_back_depth
            pops.times do
              persist_depth
              stack.pop
            end
          end
        end

        parent = stack.last[:obj]
        position = stack.last[:pos] + 1

        sec_title = norm.xpath(".//gliederungstitel/text()")[0]

        atom = parent.code_atoms.create({
          :position => position,
          :abs_position => next_pos,
          :key => gb.text,
          :code_id => code.id,
          :atom_type => CodeAtom::TYPE_ABSCHNITT,
          :depth => stack.length,
          :text => sec_title ? sec_title.text : nil
        })

        stack.last[:pos] = position
        stack.push({ :obj => atom, :pos => 0, :sec_depth => sec_depth, :depth => 0 })

        return true
      end

      false
    end

    # In rare cases, we have things like sections being paragraphs at the same
    # time. Sometimes even without "§" but just some text token.
    #--------------------------------------------------------------------------
    def scan_section_text_combo(norm)
      unless scan_for_paragraph(norm)
        enbez = norm.xpath(".//enbez/text()")[0]
        scan_for_paragraph(norm, enbez.text)
      else
        post_paragraph_work(norm)
      end
    end

    # As the section hierarchy seems to be always the same, some code only use
    # some of them, leaving gaps. So the strict difference leads to too many
    # stack pops. Instead, count upwards until we reach `low' while descending
    # `stack'.
    #--------------------------------------------------------------------------
    def stack_aware_sec_diff(high, low, stack)
      rev_stack = stack.reverse
      last_depth = high
      diff = 0

      if high == low
        return 0
      end

      rev_stack.each_index do |i|
        obj = rev_stack[i][:obj]

        # Exclude first element (high)
        if i > 0
          if obj.atom_type == CodeAtom::TYPE_ABSCHNITT
            depth = rev_stack[i][:sec_depth]

            # If we reached low, we know how much diff
            if depth == low
              diff += 1
              break
            # otherwise, count while strictly asking for something superior
            # (lower depth)
            elsif depth < last_depth
              last_depth = depth
              diff += 1
            else
              # Beyond ...
              break
            end
          # Something else we don't handle yet: stop
          else
            break
          end
        end
      end

      diff
    end

    #--------------------------------------------------------------------------
    def classify_section(text)
      # out of order :( ... What else?
      if ["jgg", "wdo"].include?(code.short_sort)
        if text.downcase == "Abschnitt"
          return SECTION_HIERARCHY.index("Kapitel")
        end
      end

      level = SECTION_HIERARCHY.index { |t| text.downcase.index(t.downcase) }
      unless level
        if text =~ /\A[IVXLC][IVXLCa-z]*\.?/
          level = SECTION_HIERARCHY.index("Roman")
        elsif text =~ /\A\d\s*[\da-z]*\./
          level = SECTION_HIERARCHY.index("Arabic")
        elsif text =~ /\A\w[\w\)]*\.?\z/
          level = SECTION_HIERARCHY.index("Letter")
        elsif text =~ /\A-\z/
          level = SECTION_HIERARCHY.index("Superior")
        # EUBeitrG
        elsif text =~ /\AAbschitt/ || text =~ /Abschnnitt/
          level = SECTION_HIERARCHY.index("Abschnitt")
        elsif code.short_sort == "stgb"
          # "Freiheitsregelnde Maßnahmen"
          level = SECTION_HIERARCHY.index("Untertitel")
        elsif code.short_sort == "wdo"
          # "Einleitende Bestimmungen"
          level = SECTION_HIERARCHY.index("Untertitel")
        elsif ["zpubkhaagg", "bmpzversano", "bvg", "bwkausl", "erbstdv"].include?(code.short_sort)
          # irregular things
          level = SECTION_HIERARCHY.index("Kapitel")
        end
      end

      level
    end

    # Appendix and specialties.
    #--------------------------------------------------------------------------
    def scan_for_appendix(norm)
      appendix = norm.xpath("./metadaten/enbez/text()")[0]

      if appendix
        title = []
        norm.xpath("./metadaten/titel/text()").each { |t| title << t }
        title = title.join(" ")

        match = APPENDIX_RE.match(appendix.text)

        if match
          shift_back_depth

          while stack.length > 1
            persist_depth
            stack.pop
          end

          parent = stack.last[:obj]
          position = stack.last[:pos] + 1

          atom = parent.code_atoms.create({
            :position => position,
            :abs_position => next_pos,
            :key => match[1],
            :code_id => code.id,
            :atom_type => CodeAtom::TYPE_ANHANG,
            :depth => stack.length,
            :text => title
          })

          create_virtual_link(atom)

          stack.last[:pos] = position
          stack.push({ :obj => atom, :pos => 0, :depth => 0 })

          content = norm.xpath("./textdaten/text/Content")[0]

          if content
            ps = content.xpath("./P")
            ps.each { |p| scan_for_appendix_elements(p) }
          end
        end
      end
    end

    # Just iterate over everything we have seen in there.
    #--------------------------------------------------------------------------
    def scan_for_appendix_elements(p)
      p.xpath("./*").each do |p_el|
        if p_el.name == "table"
          scan_table(p_el)
        end
      end
    end

    #--------------------------------------------------------------------------
    def extract_footnotes(fn, n)
      position = stack.last[:pos] + 1
      parent = stack.last[:obj]

      fn_atom = parent.code_atoms.create({
        :position => position,
        :abs_position => next_pos,
        :key => "FN #{n}",
        :code_id => code.id,
        :atom_type => CodeAtom::TYPE_FUSSNOTE,
        :depth => stack.length,
        :text => nil
      })
      stack.push({ :obj => fn_atom, :pos => 0, :depth => 1 })

      i = 1
      fn.xpath("./text()").each do |t|
        fn_atom.code_atoms.create({
          :position => i,
          :abs_position => next_pos,
          :key => i,
          :code_id => code.id,
          :atom_type => CodeAtom::TYPE_SATZ,
          :depth => stack.length,
          :text => t.text
        })
        i += 1
      end

      persist_depth
      shift_back_depth
      stack.pop

      stack.last[:pos] = position
    end

    # Takes a table with all rows and cells.
    #--------------------------------------------------------------------------
    def scan_table(table)
      parent = stack.last[:obj]
      tbl_position = stack.last[:pos] + 1

      table_atom = parent.code_atoms.create({
        :position => tbl_position,
        :abs_position => next_pos,
        :key => "Tabelle",
        :code_id => code.id,
        :atom_type => CodeAtom::TYPE_TABLE,
        :depth => stack.length,
        :text => nil
      })

      stack.last[:pos] = tbl_position
      stack.push({ :obj => table_atom, :pos => 0, :depth => 0 })
      rows = 1

      # Regardless of groups - all into one table?
      table.xpath("./tgroup").each do |grp|
        # Array of columns that act like an ID.
        as_keys = []
        needs_head_scan = true

        # Sigh, some have them, some don't. ... If they have one, we take the
        # head cells to look for ID-like columns.
        grp.xpath("./thead").each do |head|
          row = head.xpath("./row")[0]
          if row
            cells = row.xpath("./entry")
            as_keys = Array.new(cells.count, false)
            i = 0

            cells.each do |cell|
              needs_head_scan = false
              text = []
              cell.xpath("./text()").each { |t| text << t }
              text = text.join(" ")

              if APPENDIX_ID_LIKE.include?(text)
                as_keys[i] = true
              end

              i += 1
            end
          end
        end

        grp.xpath("./tbody").each do |body|
          last_key = nil
          body.xpath("./row").each do |row|
            row_atom = table_atom.code_atoms.create({
              :position => rows,
              :abs_position => next_pos,
              :key => rows,
              :code_id => code.id,
              :atom_type => CodeAtom::TYPE_TABLE_ROW,
              :depth => stack.length,
              :text => nil
            })
            entries = 1
            stack.push({ :obj => row_atom, :pos => 0, :depth => 0 })

            cells = row.xpath("./entry")
            if rows == 1 && needs_head_scan
              as_keys = Array.new(cells.count, false)
            end

            cells.each do |entry|
              text = nil
              # Take the first cells of the first row to scan for IDs unless
              # already performed
              if rows ==  1 && needs_head_scan
                text = []
                entry.xpath("./text()").each { |t| text << t }
                text = text.join(" ")

                if APPENDIX_ID_LIKE.include?(text)
                  as_keys[entries-1] = true
                end
              end

              cell_atom = row_atom.code_atoms.create({
                :position => entries,
                :abs_position => next_pos,
                :key => "Z-#{entries}",
                :code_id => code.id,
                :atom_type => CodeAtom::TYPE_TABLE_CELL,
                :depth => stack.length
              })

              stack.push({ :obj => cell_atom, :pos => 0, :as_key => last_key, :depth => 0 })

              # Set them as index ONLY IF the number of entries matches
              # and we're in the right column.
              if as_keys[entries-1] && cells.count == as_keys.count
                key = entry.xpath("./text()")[0].to_s
                cell_atom.update_attributes({
                  :key => key
                })
                # Link: Cell <-> Appendix
                create_cell_appendix_link(cell_atom)
                # Set the key here as it holds for sibling cells and sometimes
                # multiple rows until replaced
                last_key = cell_atom
              end

              # Phrases ...
              scan_for_phrases(entry)

              shift_back_depth
              persist_depth
              stack.pop

              entries += 1
            end

            rows += 1
            shift_back_depth
            persist_depth
            stack.pop
          end
        end
      end

      shift_back_depth
      persist_depth
      stack.pop
    end

    # Connect an index-like cell to the appendix.
    #--------------------------------------------------------------------------
    def create_cell_appendix_link(cell)
      reverse_stack = stack.reverse
      index = reverse_stack.index { |s| s[:obj].atom_type == CodeAtom::TYPE_ANHANG }

      if index
        parent = reverse_stack[index][:obj]
        parent.code_atom_relations.create({
          :code_atom_id => cell.id,
          :virtual => true
        })
      end
    end

    # This method scans for paragraphs and handles ranges. This one also goes
    # deeper scanning its children for actual content.
    #--------------------------------------------------------------------------
    def scan_for_paragraph(norm, name_override=nil)
      paragraph = norm.xpath("./metadaten/enbez/text()")[0]

      if paragraph
        range = []
        title_text = norm.xpath("./metadaten/titel/text()")[0]
        is_art = false

        # Get range from RE or just add a single value.
        match = PARAGRAPH_RANGED_RE.match(paragraph.text)
        match2 = ARTIKEL_RANGED_RE.match(paragraph.text)

        if match2
          match = match2
          is_art = true
        end

        if match
          range = (match[1]..match[2]).to_a
        else
          match = PARAGRAPH_RE.match(paragraph.text)
          match2 = ARTIKEL_RE.match(paragraph.text)
          if match2
            match = match2
            is_art = true
          end

          if match
            range = [match[1]]
          end
        end

        if !range.empty? || name_override
          if name_override
            range = [name_override]
          end
          range.each do |paragraph|
            # Check whether we just left a prev. paragraph.
            if dump_paragraph?
              stack.pop
            end

            parent = stack.last[:obj]
            position = stack.last[:pos] + 1

            atom = parent.code_atoms.create({
              :position => position,
              :abs_position => next_pos,
              :key => paragraph,
              :atom_type => is_art ? CodeAtom::TYPE_ARTIKEL : CodeAtom::TYPE_PARAGRAPH,
              :code_id => code.id,
              :depth => stack.length,
              :text => title_text ? title_text.text : ""
            })

            # Connecting paragraph and root.
            create_virtual_link(atom)

            stack.last[:pos] = position
            stack.push({ :obj => atom, :pos => 0, :depth => 0 })

            content = norm.xpath("./textdaten/text/Content")[0]

            # Clauses (Absätze)? Otherwise just phrases.
            if content && !scan_for_clauses(content)
              ps = content.xpath("./P")
              ps.each { |p| scan_for_phrases(p) }
            end

            # No stack pop here, must be checked as there may be footnotes
            # which requires this § as parent, but are on the same XML
            # level
          end

          return true
        end
      end

      false
    end

    #--------------------------------------------------------------------------
    def create_virtual_link(atom)
      if stack.length > 1
        # Is this ever something else than the code itself?
        # reverse_stack = stack.reverse
        # index = reverse_stack.index { |s| s[:obj].atom_type != CodeAtom::TYPE_ABSCHNITT }
        # non_section_parent = reverse_stack[index][:obj]
        non_section_parent = stack[0][:obj]

        non_section_parent.code_atom_relations.create({
          :code_atom_id => atom.id,
          :virtual => true
        })
      end
    end

    # Method to connect a given number to anything meaningful (skipping aux.
    # phrases) such as another number, clause or paragraph.
    #--------------------------------------------------------------------------
    def link_numbers(atom)
      reverse_stack = stack.reverse
      types = [CodeAtom::TYPE_NUMMER, CodeAtom::TYPE_ABSATZ, CodeAtom::TYPE_PARAGRAPH,
               CodeAtom::TYPE_ARTIKEL, CodeAtom::TYPE_TABLE_CELL]

      types.each do |type|
        index = reverse_stack.index { |s| s[:obj].atom_type == type }

        if index
          parent = nil
          # If a cell tells us to take something else, take it
          if type == CodeAtom::TYPE_TABLE_CELL && reverse_stack[index][:as_key]
            parent = reverse_stack[index][:as_key]
          else
            parent = reverse_stack[index][:obj]
          end

          parent.code_atom_relations.create({
            :code_atom_id => atom.id,
            :virtual => true
          })
          break
        end
      end
    end

    #--------------------------------------------------------------------------
    def scan_for_clauses(content)
      first_p = content.xpath("./P[1]/text()")
      has_clauses = false

      # Test whether text starts with anything like "([0-9][a-z]*)" -> clause
      if first_p
        match = ABSATZ_RE.match(first_p.text)
        has_clauses = !!match
      end

      if has_clauses
        quote_open = false
        last_clause = nil

        content.children.each do |ch|
          # regular clause
          if ch.name == "P"
            p = ch

            # § 90 AufenthG
            if p.text == ""
              next
            end

            match = ABSATZ_RE.match(p.text)
            # Test if there is something like „(x), probably destroying keys.
            # We put it into a wrapper.
            quoted = p.text.index(ABSATZ_RE_QUOT)

            if match
              parent = stack.last[:obj]
              position = stack.last[:pos] + 1

              if quoted && !quote_open
                parent = parent.code_atoms.create({
                  :position => position,
                  :abs_position => next_pos,
                  :key => "Q",
                  :code_id => code.id,
                  :depth => stack.length,
                  :atom_type => CodeAtom::TYPE_QUOTE
                })
                quote_open = true

                stack.last[:pos] = position
                stack.push({ :obj => parent, :pos => 0, :depth => 0 })
                position = 1
              end

              atom = parent.code_atoms.create({
                :position => position,
                :abs_position => next_pos,
                :key => match[1],
                :code_id => code.id,
                :atom_type => CodeAtom::TYPE_ABSATZ,
                :depth => stack.length
              })

              stack.last[:pos] = position
              stack.push({ :obj => atom, :pos => 0, :depth => 0 })

              scan_for_phrases(p)

              # Test whether there is "</..></P> here and get rid of the quote
              if quote_open
                if p.to_s.index(ABSATZ_RE_QUOT_END)
                  persist_depth
                  shift_back_depth
                  stack.pop
                  quote_open = false
                end
              else
                close_title_quote(p)
              end

              persist_depth
              shift_back_depth
              last_clause = stack.pop
            else
              # Maybe text that got lost (§ 144 AO), try to move it back in here
              match = ABSATZ_RE_RESCUE.match(p.text)
              if match && last_clause
                stack.push(last_clause)
                scan_for_phrases(p, last_clause[:pos] + 1)
                persist_depth
                shift_back_depth
                stack.pop
              elsif p.text[0] == "\""
                # § 6 SachvPruefV
                parent = stack.last[:obj]
                position = stack.last[:pos] + 1

                quote = parent.code_atoms.create({
                  :position => position,
                  :abs_position => next_pos,
                  :key => "Q",
                  :code_id => code.id,
                  :depth => stack.length,
                  :atom_type => CodeAtom::TYPE_QUOTE
                })

                stack.last[:pos] = position
                stack.push({ :obj => quote, :pos => 0, :depth => 0 })

                quote.code_atoms.create({
                  :position => position,
                  :abs_position => next_pos,
                  :key => "1",
                  :code_id => code.id,
                  :atom_type => CodeAtom::TYPE_SATZ,
                  :depth => stack.length,
                  :joined => false,
                  :text => p.text
                })

                shift_back_depth
                persist_depth
                stack.pop
              else
                raise "Absatz not matching ABSATZ_RE: #{p.text}"
              end
            end
          # Title
          elsif ch.name == "Title"
            check_for_quote(ch)
          end
        end

        return true
      end

      false
    end

    #--------------------------------------------------------------------------
    def check_for_quote(title)
      text = []
      title.xpath("./text()").each { |t| text << t }
      text = text.join(" ")

      if text[0] == "„"
        parent = stack.last[:obj]
        position = stack.last[:pos] + 1

        parent = parent.code_atoms.create({
          :position => position,
          :abs_position => next_pos,
          :key => "Q",
          :code_id => code.id,
          :depth => stack.length,
          :atom_type => CodeAtom::TYPE_QUOTE
        })

        stack.last[:pos] = position
        stack.push({ :obj => parent, :pos => 1, :depth => 1 })

        parent.code_atoms.create({
          :position => 1,
          :abs_position => next_pos,
          :key => "1",
          :code_id => code.id,
          :atom_type => CodeAtom::TYPE_SATZ,
          :depth => stack.length,
          :text => text
        })
      end
    end

    # Close this if there is quote right before a P and pop it if the quote
    # seems to end by this p.
    #--------------------------------------------------------------------------
    def close_title_quote(p)
      if stack.last[:obj].atom_type == CodeAtom::TYPE_ABSATZ &&
        stack[-2][:obj].atom_type == CodeAtom::TYPE_QUOTE
        if p.to_s.index(ABSATZ_RE_QUOT_END)
          persist_depth
          shift_back_depth
          stack.pop
        end
      end
    end

    #--------------------------------------------------------------------------
    def scan_for_phrases(p, shift=1)
      # Test whether code is SUP'ed (phrases mostly identifiable).
      sups = p.xpath("./SUP")

      unless sups.empty?
        p = clear_sups(p)
      end
      extract_unsuped_phrases(p, shift)
    end

    #--------------------------------------------------------------------------
    def clear_sups(p)
      p.xpath(".//SUP").each { |n| n.remove }
      p
    end

    # All the logic to extract individual phrases from a given `p'. `shift' can
    # be modified if the next phrase is not 1.
    #--------------------------------------------------------------------------
    def extract_unsuped_phrases(p, shift=1)
      text_repr = p.to_s
      # Mask primitive patterns from raw XML.
      masked = mask_nonphrase_stops(text_repr)

      # Get positions from where to extract phrase candidates from raw XML.
      masked_phrase_pos, sublists = sublist_masked_phrase_pos(masked)
      masked_phrase_pos = [-1] + masked_phrase_pos
      i = 1
      phrases = []

      # Collect all partials.
      while i < masked_phrase_pos.length
        start = masked_phrase_pos[i-1] + 1
        _end = masked_phrase_pos[i] - 1
        phrases << [text_repr[start..(_end)], start, _end]
        i += 1
      end

      parent = stack.last[:obj]
      xml_lists = 0

      phrases.each_index do |i|
        position = stack.last[:pos] + 1
        # Get the number of lists inside this phrase.
        phrase_sublists = get_phrase_sublists(phrases[i][1], phrases[i][2], sublists)

        # Get anything before the list begins. It's all if there is none.
        text = get_phrase_before_list(phrases[i][0])
        if text == "" && phrase_sublists == 0
          next
        end

        # Re-add "." to non-terminating, list-less phrases.
        if i != phrases.length - 1 && phrase_sublists == 0
          text += "."
        end

        new_phrase = false
        # New or append?
        unless better_join_last_pharse(text)
          new_phrase = true
          atom = parent.code_atoms.create({
            :position => position,
            :abs_position => next_pos,
            :key => shift + i,
            :code_id => code.id,
            :atom_type => CodeAtom::TYPE_SATZ,
            :depth => stack.length,
            :text => text
          })

          stack.last[:pos] = position
          stack.push({ :obj => atom, :pos => 0, :depth => 0 })
        end

        j = 0
        pushed = false
        # Now, go through all sublists of this phrase.
        while j < phrase_sublists
          not_last = j < phrase_sublists - 1
          # We push something in get_phrase_after_list, we need for
          # some contexts, we must clear that unless first/last.
          if j != 0 && not_last && pushed
            persist_depth
            stack.pop
            pushed = false
          end

          elem = p.xpath("./descendant::*[self::DL or self::table][#{xml_lists + j + 1}]")[0]

          if elem.name == "DL"
            # Do the lists.
            extract_unsuped_lists(elem)
            # Do anything after the list.
            terminated, pushed = get_phrase_after_list(elem, shift + i, not_last)
            shift += terminated
          elsif elem.name == "table"
            scan_table(elem)
            terminated, pushed = get_phrase_after_list(elem, shift + i, not_last)
            shift += terminated
          end

          j += 1
        end

        xml_lists += phrase_sublists

        if new_phrase
          if phrase_sublists == 0
            stack[-2][:depth] = [1, stack[-2][:depth]].max
          else
            shift_back_depth
          end

          persist_depth
          stack.pop
        end
      end

      shift + phrases.length - 1
    end

    # In some cases, we have a '"' after the '.' which may be the case in some
    # quotes. To not create a new phrase, add this token to the previous one
    #--------------------------------------------------------------------------
    def better_join_last_pharse(text)
      if ["“"].include?(text)
        prev = stack.last[:obj].real_code_atoms.order(:position => :desc).first
        if prev
          prev.update_attribute(:text, prev.text + text)
          return true
        end
      end

      false
    end

    # Get phrase positions aware of subordinate lists.
    #--------------------------------------------------------------------------
    def sublist_masked_phrase_pos(text)
      # Get the position of all outer DL-nodes.
      dls = get_outer_dls(text)
      # Mask these lists by setting them "-" completely.
      dls.each do |dl|
        text[dl[0]..dl[1]] = "-" * (dl[1] - dl[0] + 1)
      end

      # Now, find all remaining positions of "."
      return [text.to_enum(:scan, /\./).map { |_,_| $`.size } + [text.length], dls]
    end

    # Scanner for outer DL nodes in `text'. We need this as RE does not fit
    # this task. Also scans for outer tables.
    #--------------------------------------------------------------------------
    def get_outer_dls(text)
      length = text.length
      i = 0
      outer_dls = []
      # If we find some DL while depth is 0, we got something. This list will
      # be pushed to `outer_dls' with its starting position.
      depth = 0

      # Stop text 5 chars before end, "</DL>" will follow or not.
      while i < length - 6
        step = 1

        if ["<DL ", "<DL>", "<table ", "<table>"].index { |t| text[i..-1].index(t) == 0 }
          if depth == 0
            outer_dls << [i, nil]
          end
          depth += 1

          step = 4
        elsif ["</DL>", "</table>"].index { |t| text[i..-1].index(t) == 0 }
          depth -= 1
          # We are back on the floor, outer DL is over.
          if depth == 0
            outer_dls.last[1] = i+4
          elsif depth < 0
            depth = 0
          end
          step = 5
        end

        i += step
      end

      outer_dls
    end

    # Given the list `lists' of sublists, find all of them between `start' and
    # `stop'.
    #--------------------------------------------------------------------------
    def get_phrase_sublists(start, stop, lists)
      lists.select { |l| l[0] >= start && l[1] <= stop }.count
    end

    # Get the text following a list. If `continues' is set, it means that there
    # is a list following this text.
    #--------------------------------------------------------------------------
    def get_phrase_after_list(list, last_key, continues=false)
      # Allow everything
      siblings = list.xpath("./following-sibling::node()")
      text_next = nil

      # Maybe, there are "BR"s between list/table and text
      siblings.each do |sib|
        if sib.is_a?(Nokogiri::XML::Element)
          # There is neither a BR nor a text following
          unless sib.name == "BR"
            break
          end
        elsif sib.is_a?(Nokogiri::XML::Text)
          text_next = sib
          break
        end
      end

      # Nothing?
      unless text_next
        return [0, false]
      end

      # Primitive masking
      text_repr = mask_nonphrase_stops(text_next.text)
      # Test whether the previous list terminated the phrase so that this
      # one is a new one, not an adjoint one.
      if list.name == "DL"
        terminating = !list.to_s.index(/\.(?:<\/LA>\s*<\/DD>\s*<\/DL>)+\z/).nil?
      elsif list.name == "table"
        terminating = !list.to_s.index(/\.(?:<\/entry>\s*<\/row>\s*<\/tbody>\s*<\/tgroup>\s*<\/table>)+\z/).nil?
      end

      # Get the phrase text until next element.
      match = /([^<\.]+\.).*\z/.match(text_repr)
      pushed = false

      if match || continues
        persist_depth
        shift_back_depth
        stack.pop

        text = match ? match[1] : text_repr
        # Redo masked primitives.
        text.gsub!(/\*/, ".")

        position = stack.last[:pos] + 1

        atom = stack.last[:obj].code_atoms.create({
          :position => position,
          :abs_position => next_pos,
          :key => terminating ? (last_key + 1) : last_key,
          :code_id => code.id,
          :atom_type => CodeAtom::TYPE_SATZ,
          :depth => stack.length,
          :joined => !terminating,
          :text => text
        })

        stack.last[:pos] = position

        # If there will be another list, this atom is its new parent.
        if continues
          stack.push({ :obj => atom, :pos => 0, :depth => 0 })
        else
          stack.last[:depth] = [1, stack.last[:depth]].max
          stack.push({ :depth => 0 })
          pushed = true
        end
      end

      return [terminating ? 1 : 0, pushed]
    end

    # Extracts every list item from a given list.
    #--------------------------------------------------------------------------
    def extract_unsuped_lists(list)
      dts = list.xpath("./DT")

      # DTs and DDs are on the same level, DT: number, DD: text
      dts.each do |dt|
        number = dt.xpath(".//text()")[0]
        ret_shift = 0

        # In §2006 BGB, there is a numberless list.
        if number
          # Remove trailing "." or ")" of number.
          number = number.text.gsub(/\)$/, "").gsub(/\.$/, "")
        end

        dd = dt.xpath("./following-sibling::DD")[0]

        parent = stack.last[:obj]
        position = stack.last[:pos] + 1

        if number
          atom = parent.code_atoms.create({
            :position => position,
            :abs_position => next_pos,
            :key => number,
            :code_id => code.id,
            :atom_type => CodeAtom::TYPE_NUMMER,
            :text => "",
            :depth => stack.length
          })

          # Shortcut between numbers to hop over phrases.
          link_numbers(atom)

          stack.last[:pos] = position
          stack.push({ :obj => atom, :pos => 0, :depth => 0 })
        end

        # A DD usually contains a single or more LA (text).
        las = dd.xpath("./LA")
        shift = 1
        las.each do |la|
          text_repr = la.to_s

          # Is something connected to a sublist.
          if text_repr.index(/<DT/)
            # Text before
            text = get_phrase_before_list(text_repr)
            has_sublists = la.xpath(".//DL").length > 0

            # There is something before, so go this way via the phrases.
            if text != ""
              shift = scan_for_phrases(la, shift) + 1
            elsif has_sublists
              # Sometimes, the phrase is just the list without a preamble.
              parent = stack.last[:obj]
              position = stack.last[:pos] + 2

              atom = parent.code_atoms.create({
                :position => position,
                :abs_position => next_pos,
                :key => "1",
                :code_id => code.id,
                :atom_type => CodeAtom::TYPE_SATZ,
                :text => nil,
                :depth => stack.length
              })
              ret_shift += 1
              stack.last[:pos] = position
              stack.push({ :obj => atom, :pos => 0, :depth => 0 })

              # Do sublists directly.
              sub_dls = dd.xpath(".//DL")
              if sub_dls.count > 0
                sub_dls.each { |sub_dl| extract_unsuped_lists(sub_dl) }
                shift_back_depth
              else
                stack[-2][:depth] = [1, stack[-2][:depth]].max
              end

              persist_depth
              stack.pop
            end

          else
            # LA with text only (§ 18 UStG). Treat them as new phrases.
            shift = scan_for_phrases(la, shift + ret_shift) + 1
          end
        end

        if number
          persist_depth
          shift_back_depth
          stack.pop
        end
      end
    end

    # Extract the text preceeding a DL
    #--------------------------------------------------------------------------
    def get_phrase_before_list(text)
      has_list = !!(text.index("<DL"))
      # Re-Insert marker when sub list starts.
      text = text.gsub(/<DL[^<]*>/, "~")
      # Remove all tags
      text = text.gsub(/<[\/]*[^<]+>/, "")
      # remove (x) at the beginning
      text = text.gsub(/^\s*\([0-9]+[a-z]*\)/, "")
      # remove preceeding whitespace
      text = text.gsub(/^\s*/, "")
      # re-insert dots
      text = text.gsub(/\*/, ".")

      has_list ? text.slice(0, text.index("~")) : text
    end

    #--------------------------------------------------------------------------
    def link_atom_to_virtual(atom, virtual)
      if virtual
        virtual.code_atom_relations.create({ :code_atom_id => atom.id })
      end
    end

    # Test whether the last stack elem. is a paragraph.
    #--------------------------------------------------------------------------
    def dump_paragraph?
      stack.length > 1 && [CodeAtom::TYPE_PARAGRAPH, CodeAtom::TYPE_ARTIKEL].include?(stack.last[:obj].atom_type)
    end

    # Debug stuff
    #--------------------------------------------------------------------------
    def dump_stack
      puts stack.select { |s| s[:obj] } .map { |s| "#{s[:obj].id}, #{s[:obj].atom_type}: #{s[:obj].key} (#{s[:depth]})" }.join("\n")
    end

    #--------------------------------------------------------------------------
    def post_processing
      elems = code.code_atoms.where(:atom_type => [CodeAtom::TYPE_PARAGRAPH, CodeAtom::TYPE_ARTIKEL, CodeAtom::TYPE_ANHANG])
                             .count
      if elems > 0
        code.update_attribute(:size, elems)
      else
        # Get rid of it
        code.destroy
        self.code = nil
      end
    end
  end
end
