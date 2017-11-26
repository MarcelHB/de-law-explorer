# SPDX-License-Identifier: GPL-3.0
module HierarchyQueryBuilder
  def create_relation_upwards(atom, virtual=false)
    depth = atom.depth
    virtual_flag = virtual ? "'t'" : "'f'"

    rel = atom
    joins = [
      "INNER JOIN code_atom_relations car#{depth} ON code_atoms.id = car#{depth}.code_atom_id",
      "INNER JOIN code_atoms ca#{depth-1} ON car#{depth}.parent_code_atom_id = ca#{depth-1}.id " +
      "AND car#{depth}.virtual = #{virtual_flag}"
    ]
    fields = [
      "code_atoms.id AS code_atoms_depth_#{depth}_id",
      "code_atoms.atom_type AS code_atoms_depth_#{depth}_atom_type",
      "code_atoms.key AS code_atoms_depth_#{depth}_key"
    ]

    depth = atom.depth - 1

    while depth > 0
      joins += [
        "INNER JOIN code_atom_relations car#{depth} ON ca#{depth}.id = car#{depth}.code_atom_id",
        "INNER JOIN code_atoms ca#{depth-1} ON car#{depth}.parent_code_atom_id = ca#{depth-1}.id " +
        "AND car#{depth}.virtual = #{virtual_flag}"
      ]
      fields += [
        "ca#{depth}.id AS code_atoms_depth_#{depth}_id",
        "ca#{depth}.atom_type AS code_atoms_depth_#{depth}_atom_type",
        "ca#{depth}.key AS code_atoms_depth_#{depth}_key"
      ]

      depth -= 1
    end

    fields += [
      "ca#{depth}.id AS code_atoms_depth_#{depth}_id",
      "ca#{depth}.atom_type AS code_atoms_depth_#{depth}_atom_type",
      "ca#{depth}.key AS code_atoms_depth_#{depth}_key"
    ]

    CodeAtom.select(fields.join(",")).joins(joins).where("code_atoms.id = #{atom.id}").first
  end

  #----------------------------------------------------------------------------
  def atom_sibling(atom, dir=:prev)
    parent_joins = ["INNER JOIN code_atom_relations car ON(car.parent_code_atom_id = code_atoms.id)"]
    parent = CodeAtom.joins(parent_joins).where("car.code_atom_id = #{atom.id} AND car.virtual = 'f'").first

    selector = dir == :prev ? "<" : ">"
    sort = dir == :prev ? "DESC" : "ASC"
    parent.real_code_atoms.where("code_atoms.position #{selector} #{atom.position} AND code_atoms.joined = 'f'").order("code_atoms.position #{sort}").first
  end

  #----------------------------------------------------------------------------
  def query_downwards(stack)
    idx_unknown = stack.index { |s| s[2].nil? }
    last_known = stack[idx_unknown - 1]
    depth = idx_unknown

    fields = [
      "ca#{depth}.id AS code_atoms_depth_#{depth}_id",
      "ca#{depth}.atom_type AS code_atoms_depth_#{depth}_type",
    ]
    joins = [
      "LEFT JOIN code_atom_relations car#{depth} ON car#{depth}.parent_code_atom_id = code_atoms.id",
      "INNER JOIN code_atoms ca#{depth} ON car#{depth}.code_atom_id = ca#{depth}.id AND ca#{depth}.atom_type = #{sym_to_atom_type(stack[depth][1])} AND ca#{depth}.key = '#{stack[depth][0]}'"
    ]

    depth += 1
    while depth <= stack.length - 1
      fields += [
        "ca#{depth}.id AS code_atoms_depth_#{depth}_id",
        "ca#{depth}.atom_type AS code_atoms_depth_#{depth}_type",
      ]
      joins += [
        "LEFT JOIN code_atom_relations car#{depth} ON car#{depth}.parent_code_atom_id = ca#{depth-1}.id",
        "INNER JOIN code_atoms ca#{depth} ON car#{depth}.code_atom_id = ca#{depth}.id AND ca#{depth}.atom_type = #{sym_to_atom_type(stack[depth][1])} AND ca#{depth}.key = '#{stack[depth][0]}'"
      ]
      depth += 1
    end

    atom = CodeAtom.select(fields.join(",")).joins(joins).where("code_atoms.id = #{last_known[2]}").first

    if atom
      depth = idx_unknown
      attrs = atom.attributes
      while depth <= stack.length - 1
        stack[depth][2] = attrs["code_atoms_depth_#{depth}_id"]
        stack[depth][3] = attrs["code_atoms_depth_#{depth}_type"]
        depth += 1
      end
    end
  end

  #----------------------------------------------------------------------------
  def atoms_between(from, to, parent_id)
    joins = ["INNER JOIN code_atom_relations car ON car.parent_code_atom_id = #{parent_id} AND car.code_atom_id = code_atoms.id"]

    CodeAtom.select("code_atoms.id, code_atoms.key, code_atoms.atom_type").joins(joins).where(
      "code_atoms.abs_position > (SELECT cas1.abs_position FROM code_atoms cas1 WHERE cas1.id = #{from}) AND " +
      "code_atoms.abs_position < (SELECT cas2.abs_position FROM code_atoms cas2 WHERE cas2.id = #{to}) AND " +
      "code_atoms.atom_type = (SELECT cas3.atom_type FROM code_atoms cas3 WHERE cas3.id = #{from})"
    ).map { |ca| [ca.id, ca.key, ca.atom_type] }
  end

  #----------------------------------------------------------------------------
  def sym_to_atom_type(sym)
    {
      :paragraph => CodeAtom::TYPE_PARAGRAPH,
      :absatz => CodeAtom::TYPE_ABSATZ,
      :satz => CodeAtom::TYPE_SATZ,
      :nummer => CodeAtom::TYPE_NUMMER,
      :char => CodeAtom::TYPE_NUMMER,
      :anhang => CodeAtom::TYPE_ANHANG,
      :zelle => CodeAtom::TYPE_TABLE_CELL,
      :fussnote => CodeAtom::TYPE_FUSSNOTE,
      :artikel => CodeAtom::TYPE_ARTIKEL
    }[sym]
  end
end
