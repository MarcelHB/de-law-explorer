# SPDX-License-Identifier: GPL-3.0

# This class takes a German phrase, identifies all links and tries to actually
# connect them to other atoms.

module Linker
  class De
    include HierarchyQueryBuilder
    include AtomTokenizer
    @@parser = nil

    # Scanner for fast input movement
    KEYWORD_RE = /(?:§§?|Abs\.|Absatz|Absätze|Anlage|Artikel|Art\.|Satz|Sätze|Nr\.|Nummer|Buchstabe|Doppelbuchstabe)(n|es)?\s/

    ATOM_TYPE_MAP = {
      CodeAtom::TYPE_MASTER => :root,
      CodeAtom::TYPE_PARAGRAPH => :paragraph,
      CodeAtom::TYPE_ABSATZ => :absatz,
      CodeAtom::TYPE_SATZ => :satz,
      CodeAtom::TYPE_NUMMER => :nummer,
      CodeAtom::TYPE_ABSCHNITT => :abschnitt,
      CodeAtom::TYPE_FUSSNOTE => :fussnote,
      CodeAtom::TYPE_ANHANG => :anhang,
      CodeAtom::TYPE_ARTIKEL => :artikel,
      CodeAtom::TYPE_TABLE_CELL => :zelle
    }

    #--------------------------------------------------------------------------
    def initialize(phrase)
      self.phrase = phrase
    end

    #--------------------------------------------------------------------------
    def solve
      if quick_check
        parse
      end
    end

    private

    attr_accessor :phrase, :stack, :on_range, :total_offset

    #--------------------------------------------------------------------------
    def parser
      unless @@parser
        Treetop.load(File.join(File.dirname(__FILE__), "grammars", "ref_de.treetop"))
        @@parser = Grammar::RefDEParser.new
        # As there might be more to come, disable this.
        @@parser.consume_all_input = false
      end

      @@parser
    end

    #--------------------------------------------------------------------------
    def quick_check
      !!KEYWORD_RE.match(phrase.text)
    end

    # From the element, create a full hierarchy of atoms up to root.
    #--------------------------------------------------------------------------
    def resolve_hierarchy
      attrs = create_relation_upwards(phrase).attributes
      depth = 0
      levels = []

      # Map attributes to array levels.
      while depth <= phrase.depth
        levels << [
          attrs["code_atoms_depth_#{depth}_key"],
          atom_type_to_sym(attrs["code_atoms_depth_#{depth}_atom_type"]),
          attrs["code_atoms_depth_#{depth}_id"]
        ]
        depth += 1
      end

      levels
    end

    #--------------------------------------------------------------------------
    def atom_type_to_sym(at)
      ATOM_TYPE_MAP[at]
    end

    #--------------------------------------------------------------------------
    def sym_to_atom_type(at)
      # not a bijective function
      if at == :char
        CodeAtom::TYPE_NUMMER
      else
        ATOM_TYPE_MAP.invert[at]
      end
    end

    #--------------------------------------------------------------------------
    def hierarchy
      @hierarchy ||= resolve_hierarchy
    end

    # Parse the text. Look for symbols and move the text there for the grammar
    # parser, then move it as much as it consumed and start looking for symbols
    # again.
    #--------------------------------------------------------------------------
    def parse
      offset = 0
      links_with_offset = []
      self.total_offset = 0

      text = phrase.text
      while true
        text = text[offset..-1]

        if text.nil?
          break
        end

        offset = text.index(KEYWORD_RE)
        # Nothing left to do.
        if offset.nil?
          break
        end

        self.total_offset = total_offset + offset
        text = text[offset..-1]

        result = parser.parse(text)
        parsed_length = parser.index

        if parsed_length > 0
          # Retrieve the links from the result.
          links = solve_parse_tree(result, text)
          links.each do |l|
            links_with_offset << [l[0], total_offset, parsed_length]

            if l[0]
              name = link_name_from_stack(l[1], phrase.code_id)
              rev_name = rev_link_name(l[0])

              phrase.code_links.create({
                :to_code_atom_id => l[0],
                :start => total_offset,
                :stop => total_offset + parsed_length,
                :name => name,
                :rev_name => rev_name
              })
            end
          end
        else
          # Prevent dead-locks.
          parsed_length = 1
        end

        self.total_offset = total_offset + parsed_length
        offset = parsed_length
      end

      # Try to solve remainders
      solve_by_cross_refs(links_with_offset)
    end

    #--------------------------------------------------------------------------
    def link_name_from_stack(stack, other_code_id)
      elems = []

      stack.each do |e|
        t = atom_token(e[3], e[0])
        if t
          elems << t
        end
      end

      if stack.first[1] == :root && stack.first[4] && stack.first[4] != other_code_id
        code = Code.find(stack.first[4])
        if code
          elems << code.short
        end
      end

      elems.join(" ")
    end

    #--------------------------------------------------------------------------
    def rev_link_name(to_code_atom_id)
      rev_stack = hierarchy.map do |h|
        [h[0], nil, h[2], sym_to_atom_type(h[1]), phrase.code_id]
      end

      other_code_id = phrase.id
      if rev_stack.length > 0
        rev_stack.first[1] = :root
        other = CodeAtom.find(to_code_atom_id)
        other_code_id = other.code_id
      end

      link_name_from_stack(rev_stack, other_code_id)
    end

    # Sometimes, we have § 123, § 234 XYZ, both belonging to XYZ. The parse will
    # find both, but will only assign 234 to XYZ. Thus, if 123 cannot be solved
    # locally, try it again on XYZ.
    #--------------------------------------------------------------------------
    def solve_by_cross_refs(links)
      rev_links = links.reverse

      rev_links.each_index do |i|
        # Previous ID not empty!
        if !rev_links[i][0] && i > 0 && rev_links[i-1][0]
          prev_code_id = CodeAtom.select(:code_id).find(rev_links[i-1][0]).code_id
          # Different code
          if prev_code_id != phrase.code_id
            offset = rev_links[i][1]
            text = phrase.text[offset..-1]
            result = parser.parse(text)
            new_links = solve_parse_tree(result, text, prev_code_id)

            new_links.compact.each do |l|
              name = link_name_from_stack(l[1], phrase.code_id)
              rev_name = rev_link_name(l[0])

              phrase.code_links.create({
                :to_code_atom_id => l[0],
                :start => offset,
                :stop => offset + rev_links[i][2],
                :name => name,
                :rev_name => rev_name
              })
            end
          end
        end
      end
    end

    # This takes the note and the original text starting from the parser result.
    #--------------------------------------------------------------------------
    def solve_parse_tree(root, text="", code_id=nil)
      tree = clear_tree(root)

      # External thing
      if tree.sym == :code
        code_id = solve_code_name(tree, text)
        if code_id
          return keys_from_tree_ext(tree, code_id)
        end
      # Override internal tree by code id
      elsif code_id
        root = DENode.new(:code, code_id)
        root.elements = tree.elements
        return keys_from_tree_ext(root, code_id)
      else
        # Get appropriate parent between phrase and result.
        context_parent = find_context_parent(tree)
        if context_parent
          stack = [context_parent]
          return keys_from_tree(tree, stack)
        end
      end

      []
    end

    # For a given result, load the current hierarchy and interpolate between
    # this hierarchy and the type of the result.
    #--------------------------------------------------------------------------
    def find_context_parent(tree)
      levels = hierarchy.reverse
      top = tree.elements.first

      unless top
        return nil
      end

      # Special phrase-phrase thing?
      phrase_idx = phrase_with_subphrases(tree, levels)
      if phrase_idx
        return levels[phrase_idx+1]
      end

      # Simple approach to find a good parent.
      idx = levels.index do |lvl|
        lvl[1] == top.sym ||
        (top.sym == :anhang && [:paragraph, :artikel].include?(lvl[1])) ||
        ([:paragraph, :artikel].include?(top.sym) && lvl[1] == :fussnote) ||
        (top.sym == :char && lvl[1] == :nummer)
      end

      # Some recovery attempts.
      unless idx
        # Unless the number was found, try the previous one by prepending it into the tree.
        if [:nummer, :char].include?(top.sym)
          atom = atom_sibling(phrase)
          if atom
            parent = DENode.new(atom_type_to_sym(atom.atom_type), atom.key)
            parent.elements = tree.elements
            tree.elements = [parent]

            # Again...
            return find_context_parent(tree)
          else
            return nil
          end
        else
          raise "Could not find suitable parent! #{phrase.id}"
        end
      else
        # Slice all the Abschnitte between root and §/Anlage away.
        if [:paragraph, :anhang, :artikel].include?(top.sym) && idx > 0 && levels[idx+1][1] == :abschnitt
          return levels.last
        elsif [:nummer, :char].include?(top.sym)
          # Less specific than where we are: try upwards.
          if num_class(top.value) < num_class(levels[idx][0])
            idx = find_higher_num(levels, top.value, idx)

            unless idx
              raise "Could not find suitable number! #{phrase.id}"
            end
          # More specific than where we are: try previous phrase.
          elsif num_class(top.value) > num_class(levels[idx][0])
            atom = atom_sibling(phrase)
            parent = DENode.new(atom_type_to_sym(atom.atom_type), atom.key)
            parent.elements = tree.elements
            tree.elements = [parent]

            return find_context_parent(tree)
          end
        end
      end

      levels[idx+1]
    end

    # Try to find a number in `levels` (reverse hierarchy) beyond `greater_idx`
    # of the same type as `value`.
    #--------------------------------------------------------------------------
    def find_higher_num(levels, value, greater_idx)
      idx = nil
      val_class = num_class(value)

      levels.each_index do |i|
        if i > greater_idx && [:nummer, :char].include?(levels[i][1]) && val_class == num_class(levels[i][0])
          idx = i
          break
        end
      end

      idx
    end

    # Classifies number by its pattern into a depth level.
    #--------------------------------------------------------------------------
    def num_class(num)
      case num
      when /^[a-z][a-z]$/
        10
      when /^[a-z]$/
        5
      when /^[0-9]+[a-z]*$/
        1
      else
        # Whatever we have not spotted so far ...
        2
      end
    end

    # Rarely, a reference contains multiple phrases as hierarchy. We cannot
    # simply choose any of them and their parent. Instead, try to find how many
    # there are and jump the stack back accordingly.
    #--------------------------------------------------------------------------
    def phrase_with_subphrases(tree, levels)
      top = tree.elements.first
      depth = 0
      sub_depth = 0

      if top && top.sym == :satz
        tree.elements.each do |e|
          # Go over tree.
          sub_depth = [sub_depth, tree_phrase_depth(e, depth)].max
        end
      end
      depth = [sub_depth, depth].max

      if depth > 1
        i = 1
        idx = levels.index do |l|
          if l[1] == :satz
            # Found the correct depth!
            if i == depth
              true
            else
              i += 1
              nil
            end
          end
        end

        if idx
          return idx
        end
      end

      nil
    end

    # Recursive helper of the previous, returning how many phrases as in the
    # depth at max in a row.
    #--------------------------------------------------------------------------
    def tree_phrase_depth(node, depth)
      depth = node.sym == :satz ? depth + 1 : depth
      sub_depth = 0

      node.elements.each do |e|
        sub_depth = [sub_depth, tree_phrase_depth(e, depth)].max
      end

      [depth, sub_depth].max
    end

    # Root method to extract keys from a cleared tree, given aux. info from
    # stack.
    #--------------------------------------------------------------------------
    def keys_from_tree(tree, stack)
      keys = []

      tree.elements.each do |e|
        keys += keys_from_subtree(e, stack)
      end

      keys
    end

    # Same as above, but get the root of the given `code_id`.
    #--------------------------------------------------------------------------
    def keys_from_tree_ext(tree, code_id)
      stack = []
      keys = []

      top = Code.find(code_id).code_atoms.select(:id).where(:atom_type => CodeAtom::TYPE_MASTER).first
      stack << [nil, :root, top.id, nil, code_id]

      tree.elements.each do |e|
        keys += keys_from_subtree(e, stack)
      end

      keys
    end

    # Solves code name from the root of the tree. We need this for two reasons:
    #   1. Even if the parser returned anything correctly until the last char.,
    #      we still have articles in front or declinations that will prevent
    #      finding the correct code via the alias-table.
    #
    #      Example:
    #
    #        "des Bürgerlichen Gesetzbuchs" -> "Bürgerliches Gesetzbuch"
    #
    #  2. Sometimes "Gesetz" is only the beginning, but the parser stopped
    #     there. Then we need to look for everything following it until
    #     there is a match or an end. This is why we need `original_text`.
    #--------------------------------------------------------------------------
    def solve_code_name(tree, original_text="")
      calias = CodeAlias.find_by_alias(normalize_code_name(tree.value))

      unless calias
        i = (original_text.index(Regexp.new(tree.value)) || 0) + tree.value.length
        exhausted = false
        try = false
        candidate = tree.value

        while i < original_text.length && !exhausted
          # possible terminators
          if [".", ";", ",", "§", "(", ")"].include?(original_text[i])
            exhausted = true
            try = true
          elsif original_text[i] == " "
            # At any space, try another lookup.
            candidate += original_text[i]
            try = true
          else
            candidate += original_text[i]
          end

          if try
            calias = CodeAlias.find_by_alias(normalize_code_name(candidate))
            if calias
              break
            end
            try = false
          end

          i += 1
        end
      end

      calias ? Code.find(calias.code_id) : nil
    end

    # Removes articles and replaces declinations.
    #--------------------------------------------------------------------------
    def normalize_code_name(name)
      norm = name.dup
      norm.strip!
      norm.gsub!(/\A(?:der|des)\s+/, "")
      norm.gsub!(/(gesetz)es/, '\1')
      norm.gsub!(/(Gesetz)es/, '\1')
      norm.gsub!(/(buch)e?s/, '\1')
      norm.gsub!(/(Buch)e?s/, '\1')
      norm.gsub!(/en(\s+Gesetz)/, 'es\1')
      norm.gsub!(/en(\s+Buch)/, 'es\1')
      norm
    end

    # Collect all trees from a given `subtree` with the help of `stack`.
    #--------------------------------------------------------------------------
    def keys_from_subtree(tree, stack, override_sym=nil)
      keys = []
      stack.push([tree.value, override_sym || tree.sym, nil])

      # Non-terminating element
      unless tree.elements.empty?
        tree.elements.each do |e|
          # special mapping
          if tree.sym == :anhang && e.sym == :nummer
            keys += keys_from_subtree(e, stack, :zelle)
          else
            keys += keys_from_subtree(e, stack)
          end
        end
      else
        # Terminating, with range
        if tree.ranged
          keys += keys_from_path_with_range(stack, tree)
        else
          # Terminating, solve it!
          keys << key_from_path(stack)
        end
      end

      stack.pop

      keys
    end

    #--------------------------------------------------------------------------
    def key_from_path(stack)
      query_downwards(stack)
      [stack.last[2], stack.dup]
    end

    # Similar to the one above, but do two lookups: for start and end and then
    # find anything in between.
    #--------------------------------------------------------------------------
    def keys_from_path_with_range(stack, tree)
      query_downwards(stack)
      from_key = stack.last[2]

      if from_key
        to_stack = stack.dup
        to_stack.pop
        to_stack.push([tree.value_to, tree.sym, nil])
        query_downwards(to_stack)
        to_key = to_stack.last[2]

        if to_key
          atoms_btw = atoms_between(from_key, to_key, stack[-2][2])
          atoms_btw.map! do |a|
            btw_stack = stack.dup
            btw_stack.pop
            btw_stack.push([a[1], nil, a[0], a[2]])
            [a[0], btw_stack]
          end
          return [[from_key, stack.dup]] + atoms_btw + [[to_key, to_stack]]
        end
      end

      []
    end

    # Debugging
    #--------------------------------------------------------------------------
    def dump_tree(node)
      puts node.inspect
    end

    # Debugging
    #--------------------------------------------------------------------------
    def dump_cleared_tree(node, d=0)
      puts "#{" " * d} #{node.sym} #{node.value} #{node.ranged ? node.value_to : ""}"

      node.elements.each do |e|
        dump_cleared_tree(e, d + 1)
      end
    end

    # --- TRANSFORMATIONS ----------------------------------------------------
    # Transforms a given Treetop result, annotated with our own classes and
    # modules into a tree (cleared tree) representing a semantic hieararchy
    # and skipping all the garbage.

    #--------------------------------------------------------------------------
    def clear_tree(root)
      self.stack = [DENode.new(:root)]
      self.on_range = false

      case root
      when Grammar::RefDE::Paragraph
        clear_paragraph(root)
      when Grammar::RefDE::Absatz
        clear_absatz(root)
      when Grammar::RefDE::Satz
        clear_satz(root)
      when Grammar::RefDE::Nummer
        clear_nummer(root)
      when Grammar::RefDE::Anlage
        clear_anlage(root)
      when Grammar::RefDE::Artikel
        clear_artikel(root)
      when Grammar::RefDE::GesetzCode
      end

      stack.first
    end

    #--------------------------------------------------------------------------
    def clear_paragraph(node)
      node.elements.each do |e|
        if e.is_a?(Grammar::RefDE::ParagraphChars)
          clear_paragraph_chars_f(e)
          stack.pop
        end
      end
    end

    #--------------------------------------------------------------------------
    def clear_artikel(node)
      node.elements.each do |e|
        if e.is_a?(Grammar::RefDE::ArtikelChars)
          clear_artikel_chars_f(e)
          stack.pop
        end
      end
    end

    #--------------------------------------------------------------------------
    def clear_absatz(node)
      node.elements.each do |e|
        if e.is_a?(Grammar::RefDE::AbsatzChars)
          clear_absatz_chars_f(e)
          stack.pop
        end
      end
    end

    #--------------------------------------------------------------------------
    def clear_satz(node)
      node.elements.each do |e|
        if e.is_a?(Grammar::RefDE::SatzChars)
          clear_satz_chars_f(e)
          stack.pop
        end
      end
    end

    #--------------------------------------------------------------------------
    def clear_nummer(node)
      node.elements.each do |e|
        if e.is_a?(Grammar::RefDE::NummerCore)
          clear_nummer_core(e)
        elsif e.is_a?(Grammar::RefDE::NummerIAnlage)
          clear_nummer_anlage(e)
        end
      end
    end

    #--------------------------------------------------------------------------
    def clear_anlage(node)
      node.elements.each do |e|
        case e
        when Grammar::RefDE::AnlageChar
          assign_appendix(e)
        end
      end
    end

    #--------------------------------------------------------------------------
    def clear_paragraph_chars_f(node)
      case node
      when Grammar::RefDE::ParagraphSep, Grammar::RefDE::ParagraphBin
        clear_paragraph_bin(node)
      when Grammar::RefDE::ParagraphRange
        clear_paragraph_range(node)
      when Grammar::RefDE::ParagraphSub
        clear_paragraph_sub(node)
      end
    end

    #--------------------------------------------------------------------------
    def clear_artikel_chars_f(node)
      case node
      when Grammar::RefDE::ArtikelSep, Grammar::RefDE::ArtikelBin
        clear_artikel_bin(node)
      when Grammar::RefDE::ArtikelRange
        clear_artikel_range(node)
      when Grammar::RefDE::ArtikelSub
        clear_artikel_sub(node)
      when Grammar::RefDE::ArtikelChar
        consume_char(node, :artikel)
      end
    end

    #--------------------------------------------------------------------------
    def clear_absatz_chars_f(node)
      case node
      when Grammar::RefDE::AbsatzSep, Grammar::RefDE::AbsatzBin
        clear_absatz_bin(node)
      when Grammar::RefDE::AbsatzRange
        clear_absatz_range(node)
      when Grammar::RefDE::AbsatzSub
        clear_absatz_sub(node)
      when Grammar::RefDE::AbsatzChar
        consume_char(node, :absatz)
      end
    end

    #--------------------------------------------------------------------------
    def clear_satz_chars_f(node)
      case node
      when Grammar::RefDE::SatzSep, Grammar::RefDE::SatzBin
        clear_satz_bin(node)
      when Grammar::RefDE::SatzRange
        clear_satz_range(node)
      when Grammar::RefDE::SatzSub
        clear_satz_sub(node)
      when Grammar::RefDE::SatzChar
        consume_char(node, :satz)
      end
    end

    #--------------------------------------------------------------------------
    def clear_nummer_chars_f(node)
      case node
      when Grammar::RefDE::NummerSep, Grammar::RefDE::NummerBin
        clear_nummer_bin(node)
      when Grammar::RefDE::NummerRange
        clear_nummer_range(node)
      when Grammar::RefDE::NummerSub
        clear_nummer_sub(node)
      when Grammar::RefDE::NummerChar
        consume_char(node, :nummer)
      end
    end

    #--------------------------------------------------------------------------
    def clear_char_chars_f(node)
      case node
      when Grammar::RefDE::CharSep, Grammar::RefDE::CharBin
        clear_char_bin(node)
      when Grammar::RefDE::CharRange
        clear_char_range(node)
      when Grammar::RefDE::CharSub
        clear_char_sub(node)
      when Grammar::RefDE::CharChar
        consume_char(node, :char)
      end
    end

    #--------------------------------------------------------------------------
    def clear_nummer_core(node)
      node.elements.each do |e|
        case e
        when Grammar::RefDE::NummerChars
          clear_nummer_chars_f(e)
          stack.pop
        when Grammar::RefDE::CharChars
          clear_char_chars_f(e)
          stack.pop
        end
      end
    end

    #--------------------------------------------------------------------------
    def clear_nummer_anlage(node)
      node.elements.each do |e|
        case e
        when Grammar::RefDE::NummerAnlage
          clear_nummer_anlage(e)
        when Grammar::RefDE::Anlage
          clear_anlage(e)
        end
      end
    end

    #--------------------------------------------------------------------------
    def clear_paragraph_bin(node)
      node.elements.each do |e|
        case e
        when Grammar::RefDE::ParagraphChar
          consume_char(e, :paragraph)
        when Grammar::RefDE::ParagraphChars
          clear_paragraph_chars_f(e)
        end
      end
    end

    #--------------------------------------------------------------------------
    def clear_artikel_bin(node)
      node.elements.each do |e|
        case e
        when Grammar::RefDE::ArtikelChar
          consume_char(e, :artikel)
        when Grammar::RefDE::ArtikelChars
          clear_artikel_chars_f(e)
        end
      end
    end

    #--------------------------------------------------------------------------
    def clear_absatz_bin(node)
      node.elements.each do |e|
        case e
        when Grammar::RefDE::AbsatzChar
          consume_char(e, :absatz)
        when Grammar::RefDE::AbsatzISub
          clear_absatz_sub(e)
        when Grammar::RefDE::AbsatzChars
          clear_absatz_chars_f(e)
        end
      end
    end

    #--------------------------------------------------------------------------
    def clear_satz_bin(node)
      node.elements.each do |e|
        case e
        when Grammar::RefDE::SatzChar
          consume_char(e, :satz)
        when Grammar::RefDE::SatzSub
          clear_satz_sub(e)
        when Grammar::RefDE::SatzChars
          clear_satz_chars_f(e)
        end
      end
    end

    #--------------------------------------------------------------------------
    def clear_nummer_bin(node)
      node.elements.each do |e|
        case e
        when Grammar::RefDE::NummerChar
          consume_char(e, :nummer)
        when Grammar::RefDE::NummerISub
          clear_nummer_sub(e)
        when Grammar::RefDE::NummerChars
          clear_nummer_chars_f(e)
        end
      end
    end

    #--------------------------------------------------------------------------
    def clear_char_bin(node)
      node.elements.each do |e|
        case e
        when Grammar::RefDE::CharChar
          consume_char(e, :char)
        when Grammar::RefDE::CharSub
          clear_char_sub(e)
        when Grammar::RefDE::CharChars
          clear_char_chars_f(e)
        end
      end
    end

    #--------------------------------------------------------------------------
    def clear_paragraph_range(node)
      node.elements.each do |e|
        case e
        when Grammar::RefDE::ParagraphChar
          consume_char(e, :paragraph, true)
          self.on_range = true
        when Grammar::RefDE::ParagraphChars
          clear_paragraph_chars_f(e)
        end
      end
    end

    #--------------------------------------------------------------------------
    def clear_artikel_range(node)
      node.elements.each do |e|
        case e
        when Grammar::RefDE::ArtikelChar
          consume_char(e, :artikel, true)
          self.on_range = true
        when Grammar::RefDE::ArtikelChars
          clear_artikel_chars_f(e)
        end
      end
    end

    #--------------------------------------------------------------------------
    def clear_absatz_range(node)
      node.elements.each do |e|
        case e
        when Grammar::RefDE::AbsatzChar
          consume_char(e, :absatz, true)
          self.on_range = true
        when Grammar::RefDE::AbsatzChars
          clear_absatz_chars_f(e)
        end
      end
    end

    #--------------------------------------------------------------------------
    def clear_satz_range(node)
      node.elements.each do |e|
        case e
        when Grammar::RefDE::SatzChar
          consume_char(e, :satz, true)
          self.on_range = true
        when Grammar::RefDE::SatzChars
          clear_satz_chars_f(e)
        end
      end
    end

    #--------------------------------------------------------------------------
    def clear_nummer_range(node)
      node.elements.each do |e|
        case e
        when Grammar::RefDE::NummerChar
          consume_char(e, :nummer, true)
          self.on_range = true
        when Grammar::RefDE::NummerChars
          clear_nummer_chars_f(e)
        end
      end
    end

    #--------------------------------------------------------------------------
    def clear_char_range(node)
      node.elements.each do |e|
        case e
        when Grammar::RefDE::CharChar
          consume_char(e, :char, true)
          self.on_range = true
        when Grammar::RefDE::CharChars
          clear_char_chars_f(e)
        end
      end
    end

    #--------------------------------------------------------------------------
    def clear_paragraph_sub(node)
      node.elements.each do |e|
        case e
        when Grammar::RefDE::ParagraphChar
          consume_char(e, :paragraph)
        when Grammar::RefDE::ParagraphSubs
          clear_paragraph_subs(e)
        end
      end
    end

    #--------------------------------------------------------------------------
    def clear_artikel_sub(node)
      node.elements.each do |e|
        case e
        when Grammar::RefDE::ArtikelChar
          consume_char(e, :artikel)
        when Grammar::RefDE::ArtikelSubs
          clear_artikel_subs(e)
        end
      end
    end

    #--------------------------------------------------------------------------
    def clear_absatz_sub(node)
      node.elements.each do |e|
        case e
        when Grammar::RefDE::AbsatzChar
          consume_char(e, :absatz)
        when Grammar::RefDE::AbsatzSubs
          clear_absatz_subs_f(e)
        end
      end
    end

    #--------------------------------------------------------------------------
    def clear_satz_sub(node)
      node.elements.each do |e|
        case e
        when Grammar::RefDE::SatzChar
          consume_char(e, :satz)
        when Grammar::RefDE::SatzSubs
          clear_satz_subs_f(e)
        end
      end
    end

    #--------------------------------------------------------------------------
    def clear_nummer_sub(node)
      node.elements.each do |e|
        case e
        when Grammar::RefDE::NummerChar
          consume_char(e, :nummer)
        when Grammar::RefDE::NummerSubs
          clear_numchar_subs_f(e)
        end
      end
    end

    #--------------------------------------------------------------------------
    def clear_char_sub(node)
      node.elements.each do |e|
        case e
        when Grammar::RefDE::CharChar
          consume_char(e, :char)
        when Grammar::RefDE::CharSubs
          clear_numchar_subs_f(e)
        end
      end
    end

    #--------------------------------------------------------------------------
    def clear_paragraph_subs(node)
      node.elements.each do |e|
        case e
        when Grammar::RefDE::ParagraphSubsType
          clear_paragraph_subs_type_f(e)
        when Grammar::RefDE::ParagraphSubsGesetz
          clear_paragraph_subs_gesetz(e)
        end
      end
    end

    #--------------------------------------------------------------------------
    def clear_artikel_subs(node)
      node.elements.each do |e|
        case e
        when Grammar::RefDE::ArtikelSubsType
          clear_artikel_subs_type_f(e)
        when Grammar::RefDE::ArtikelSubsGesetz
          clear_artikel_subs_gesetz(e)
        end
      end
    end

    #--------------------------------------------------------------------------
    def clear_paragraph_subs_type_f(node)
      case node
      when Grammar::RefDE::ParagraphSubsAbs, Grammar::RefDE::ParagraphSubsSatz, Grammar::RefDE::ParagraphSubsNum
        clear_paragraph_isubs(node)
      end
    end

    #--------------------------------------------------------------------------
    def clear_artikel_subs_type_f(node)
      case node
      when Grammar::RefDE::ArtikelSubsAbs, Grammar::RefDE::ArtikelSubsSatz, Grammar::RefDE::ArtikelSubsNum
        clear_artikel_isubs(node)
      end
    end

    #--------------------------------------------------------------------------
    def clear_paragraph_subs_gesetz(node)
      node.elements.each do |e|
        case e
        when Grammar::RefDE::GesetzCode
          assign_code(e)
        end
      end
    end

    #--------------------------------------------------------------------------
    def clear_artikel_subs_gesetz(node)
      node.elements.each do |e|
        case e
        when Grammar::RefDE::GesetzCode
          assign_code(e)
        end
      end
    end

    #--------------------------------------------------------------------------
    def clear_paragraph_isubs(node)
      node.elements.each do |e|
        case e
        when Grammar::RefDE::Absatz
          clear_absatz(e)
        when Grammar::RefDE::Satz
          clear_satz(e)
        when Grammar::RefDE::Nummer
          clear_nummer(e)
        end
      end
    end

    #--------------------------------------------------------------------------
    def clear_artikel_isubs(node)
      node.elements.each do |e|
        case e
        when Grammar::RefDE::Absatz
          clear_absatz(e)
        when Grammar::RefDE::Satz
          clear_satz(e)
        when Grammar::RefDE::Nummer
          clear_nummer(e)
        end
      end
    end

    #--------------------------------------------------------------------------
    def clear_absatz_subs_f(node)
      case node
      when Grammar::RefDE::Nummer
        clear_nummer(node)
      when Grammar::RefDE::Satz
        clear_satz(node)
      end
    end

    #--------------------------------------------------------------------------
    def clear_satz_subs_f(node)
      case node
      when Grammar::RefDE::Nummer
        clear_nummer(node)
      end
    end

    #--------------------------------------------------------------------------
    def clear_numchar_subs_f(node)
      case node
      when Grammar::RefDE::Satz
        clear_satz(node)
      when Grammar::RefDE::Nummer
        clear_nummer(node)
      end
    end

    # Extract a given key with respect to ranges.
    #--------------------------------------------------------------------------
    def consume_char(node, sym, ranged=false)
      if on_range
        prev = stack.last
        prev.value_to = node.text_value
        self.on_range = false
      else
        if stack.last.sym == sym
          stack.pop
        end
        prev = stack.last
        pc = DENode.new(sym, node.text_value, ranged)
        prev.elements << pc
        stack.push(pc)
      end
    end

    # Prepend code into tree
    #--------------------------------------------------------------------------
    def assign_code(node)
      code = DENode.new(:code, node.text_value)
      root = stack.first
      code.elements = root.elements
      stack[0] = code
    end

    #--------------------------------------------------------------------------
    def assign_appendix(node)
      appdx = DENode.new(:anhang, node.text_value)
      root = stack.first
      appdx.elements = root.elements
      root.elements = [appdx]
      stack.insert(1, appdx)
    end
  end

  # Auxiliary structure to represent the cleared tree.
  #----------------------------------------------------------------------------
  class DENode
    attr_accessor :elements, :sym, :value, :value_to, :ranged

    def initialize(sym, value=nil, ranged=false)
      self.sym = sym
      self.value = value
      self.value_to = nil
      self.elements = []
      self.ranged = ranged
    end
  end
end

# --- TREETOP MARKERS ---------------------------------------------------------
# This is what we need to mark some of the TT rules as rule or terminal in a
# way we want to identify it.

module Grammar
  module RefDE
    REF_DE_MODS = %w(
      Paragraph
      ParagraphChars
      ParagraphSubs
      ParagraphSubsAbs
      ParagraphSubsAbsPre
      ParagraphSubsNum
      ParagraphSubsSatz
      ParagraphSubsGesetz
      ParagraphSubsType
      Absatz
      AbsatzChars
      AbsatzSubs
      Satz
      SatzChars
      SatzSubs
      Nummer
      NummerChars
      NummerCore
      NummerSubs
      CharChars
      CharSubs
      Artikel
      ArtikelChars
      ArtikelSubs
      ArtikelSubsAbs
      ArtikelSubsAbsPre
      ArtikelSubsNum
      ArtikelSubsSatz
      ArtikelSubsGesetz
      ArtikelSubsType
      GesetzCode
      Anlage
      AnlageChars
    )

    REF_DE_CLASSES = %w(
      BinOp
      TextMarker
      ParagraphChar
      ParagraphSep
      ParagraphBin
      ParagraphRange
      ParagraphSub
      AbsatzChar
      AbsatzBin
      AbsatzSep
      AbsatzRange
      AbsatzISub
      AbsatzSub
      SatzChar
      SatzSep
      SatzRange
      SatzBin
      SatzISub
      SatzSub
      NummerChar
      NummerSep
      NummerBin
      NummerRange
      NummerSub
      NummerISub
      NummerAnlage
      NummerIAnlage
      CharSep
      CharChar
      CharBin
      CharSub
      CharISub
      CharRange
      ArtikelChar
      ArtikelSub
      ArtikelSep
      ArtikelBin
      ArtikelRange
      AnlageChar
      Indenter
    )
  end
end

Grammar::RefDE::REF_DE_MODS.each do |name|
  mod = Module.new
  Grammar::RefDE.const_set(name, mod)
end

Grammar::RefDE::REF_DE_CLASSES.each do |name|
  cl = Class.new(Treetop::Runtime::SyntaxNode)
  Grammar::RefDE.const_set(name, cl)
end
