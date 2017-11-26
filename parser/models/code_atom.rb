# SPDX-License-Identifier: GPL-3.0
class CodeAtom < ActiveRecord::Base
  belongs_to  :code

  has_many    :code_atom_relations, :foreign_key => :parent_code_atom_id, :dependent => :destroy
  has_many    :code_atoms, :through => :code_atom_relations, :dependent => :destroy
  has_many    :real_code_atoms, Proc.new{ where("code_atom_relations.virtual = 'f'") }, :through => :code_atom_relations, :source => :code_atom, :class_name => "CodeAtom"
  has_many    :code_links, :dependent => :destroy
  has_many    :from_code_links, :class_name => "CodeLink", :foreign_key => :to_code_atom_id, :dependent => :destroy

  scope :depth, Proc.new { |d| where(:depth => d) }
  scope :type, Proc.new { |t| where(:atom_type => t) }

  TYPE_MASTER = 0
  TYPE_ABSCHNITT = 1
  TYPE_PARAGRAPH = 2
  TYPE_ARTIKEL = 3
  TYPE_ABSATZ = 4
  TYPE_SATZ = 5
  TYPE_NUMMER = 6
  TYPE_ANHANG = 7
  TYPE_FUSSNOTE = 8
  TYPE_TABLE = 9
  TYPE_TABLE_ROW = 10
  TYPE_TABLE_CELL = 11
  TYPE_QUOTE = 12

  before_save :clear_linebreaks

  #----------------------------------------------------------------------------
  def self.create_table(b)
    b.create_table :code_atoms do |t|
      t.integer   :code_id, :null => false
      t.integer   :position, :null => false, :default => 0
      t.integer   :abs_position, :null => false, :default => 0
      t.string    :key, :null => false
      t.integer   :atom_type, :null => false
      t.integer   :depth, :null => false, :default => 0
      t.integer   :tree_depth, :null => false, :default => 0
      t.boolean   :joined, :default => false
      t.text      :text
    end
  end

  #----------------------------------------------------------------------------
  def self.add_table_keys(b)
    b.add_index   :code_atoms, [:atom_type, :key]
    b.add_index   :code_atoms, [:code_id]
    b.add_index   :code_atoms, [:code_id, :atom_type, :abs_position]
  end

  private

  #----------------------------------------------------------------------------
  def clear_linebreaks
    if text
      cstr = text.gsub(/\r?\n/, " ")
      self.text = cstr.strip
    end
  end
end
