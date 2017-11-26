# SPDX-License-Identifier: GPL-3.0
class CodeLink < ActiveRecord::Base
  belongs_to  :code_atom
  belongs_to  :to_code_atom, :class_name => "CodeAtom"

  #----------------------------------------------------------------------------
  def self.create_table(b)
    b.create_table :code_links do |t|
      t.integer   :code_atom_id, :null => false
      t.integer   :to_code_atom_id, :null => false
      t.string    :name
      t.string    :rev_name
      t.integer   :start, :null => false
      t.integer   :stop, :null => false
    end
  end

  #----------------------------------------------------------------------------
  def self.add_table_keys(b)
    b.add_index   :code_links, [:code_atom_id]
    b.add_index   :code_links, [:to_code_atom_id]
  end

  #----------------------------------------------------------------------------
  def clear_linebreaks
    if name
      cstr = name.gsub(/\r?\n/, " ")
      self.name = cstr.strip
    end
  end
end
