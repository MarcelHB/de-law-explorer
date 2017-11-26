# SPDX-License-Identifier: GPL-3.0
class CodeAtomRelation < ActiveRecord::Base
  belongs_to  :parent_code_atom, :class_name => "CodeAtom"
  belongs_to  :code_atom

  #----------------------------------------------------------------------------
  def self.create_table(b)
    b.create_table :code_atom_relations do |t|
      t.integer   :parent_code_atom_id, :null => false
      t.integer   :code_atom_id, :null => false
      t.boolean   :virtual, :default => false
    end
  end

  #----------------------------------------------------------------------------
  def self.add_table_keys(b)
    b.add_index   :code_atom_relations, [:code_atom_id]
    b.add_index   :code_atom_relations, [:parent_code_atom_id, :virtual]
  end
end
