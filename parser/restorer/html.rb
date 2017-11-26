# SPDX-License-Identifier: GPL-3.0

module Restorer
  class HTML
    #--------------------------------------------------------------------------
    def initialize(code)
      self.code = code
    end

    #--------------------------------------------------------------------------
    def action(path)
      self.path = path

      create_out_header
      write_atoms(code.code_atoms.type(CodeAtom::TYPE_MASTER).first)
      create_out_footer
    end

    private

    attr_accessor :code, :path

    #--------------------------------------------------------------------------
    def out
      @out_file ||= File.open(path, "w")
    end

    #--------------------------------------------------------------------------
    def write_atoms(parent)
      if parent.atom_type != CodeAtom::TYPE_MASTER
        atom = <<-HTML
          <tr id="c#{parent.id}">
            <td>#{parent.id}</td>
            <td>#{parent.depth}</td>
            <td>#{parent.tree_depth}</td>
            <td>#{typ_str(parent)}</td>
            <td>#{parent.key}</td>
            <td style="padding-left: #{10 * parent.depth}px">#{text_with_links(parent)}</td>
          </tr>
        HTML
        out.write(atom)
      end

      parent.real_code_atoms.order("position ASC").each do |atom|
        write_atoms(atom)
      end
    end

    #--------------------------------------------------------------------------
    def typ_str(atom)
      {
        CodeAtom::TYPE_ABSCHNITT => "Abschnitt",
        CodeAtom::TYPE_PARAGRAPH => "§",
        CodeAtom::TYPE_ARTIKEL => "Art.",
        CodeAtom::TYPE_ABSATZ => "Abs.",
        CodeAtom::TYPE_SATZ => "Satz",
        CodeAtom::TYPE_NUMMER => "Nr.",
        CodeAtom::TYPE_ANHANG => "Anhang",
        CodeAtom::TYPE_FUSSNOTE => "FN",
        CodeAtom::TYPE_TABLE => "Tbl.",
        CodeAtom::TYPE_TABLE_ROW => "Rh.",
        CodeAtom::TYPE_TABLE_CELL => "Zelle",
        CodeAtom::TYPE_QUOTE => "Quote"
      }[atom.atom_type]
    end

    #--------------------------------------------------------------------------
    def create_out_header
      header = <<-HTML
      <!DOCUMENT html>
      <html>
        <head>
          <title>#{code.name}</title>
          <meta charset="UTF-8">
          <style type="text/css">
              table.atoms td { vertical-align: top; }
          </style>
        </head>
        <body>
          <h2>#{code.name} (#{code.short})</h2>
          <table class="atoms">
            <tr>
              <th>ID</th>
              <th>Tiefe</th>
              <th>Baumtiefe</th>
              <th>Typ</th>
              <th>Schlüssel</th>
              <th>Text</th>
            </tr>
      HTML

      out.write(header)
    end

    #--------------------------------------------------------------------------
    def create_out_footer
      header = <<-HTML
          </table>
        </body>
      </html>
      HTML

      out.write(header)
    end

    #--------------------------------------------------------------------------
    def text_with_links(node)
      raw_text = node.text
      links = node.code_links.order(:start => :asc)

      if links.count > 0
        linked_text = []

        snippets, from_last = insert_links(links, raw_text)
        linked_text += snippets

        linked_text << (from_last >= raw_text.length ? "" : raw_text[from_last..-1])
        linked_text.join("")
      else
        raw_text
      end
    end

    #--------------------------------------------------------------------------
    def insert_links(links, text)
      buckets = {}

      links.each do |link|
        buckets[link.start] ||= []
        buckets[link.start] << link
      end

      last = 0
      snippets = []

      buckets.each_pair do |k,v|
        if k > 0
          snippets << text[last..k-1]
        end
        to = buckets[k].map { |l| l.stop }.max
        snippets << "<a href=\"\">" + text[k..to-1] + "</a> "

        post_links = []
        v.each do |link|
          post_links << "<a href=\"#c#{link.to_code_atom_id}\">##{link.to_code_atom_id}</a>"
        end
        snippets << "(" + post_links.join(", ") + ")"

        last = to
      end

      [snippets, last]
    end
  end
end
