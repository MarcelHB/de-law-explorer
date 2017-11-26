# SPDX-License-Identifier: GPL-3.0
class CodeAlias < ActiveRecord::Base
  belongs_to :code

  before_save :clear_linebreaks

  #----------------------------------------------------------------------------
  def self.create_table(b)
    b.create_table :code_aliases do |t|
      t.string    :alias, :limit => 500, :null => false
      t.integer   :code_id, :null => false
    end
  end

  #----------------------------------------------------------------------------
  def self.add_table_keys(b)
    b.add_index :code_aliases, :alias
  end

  private

  #----------------------------------------------------------------------------
  def clear_linebreaks
    if self.alias
      cstr = self.alias.gsub(/\r?\n/, " ")
      self.alias = cstr.strip
    end
  end
end
