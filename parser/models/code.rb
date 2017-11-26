# SPDX-License-Identifier: GPL-3.0
class Code < ActiveRecord::Base
  belongs_to  :domain

  has_many :code_aliases, :dependent => :destroy
  has_many :code_atoms,   :dependent => :destroy
  has_many :inter_code_links, :dependent => :destroy
  has_many :to_inter_code_links, :class_name => "InterCodeLink", :foreign_key => :to_code_id, :dependent => :destroy

  before_save   :clear_linebreaks
  before_save   :make_sortable_short

  #----------------------------------------------------------------------------
  def self.create_table(b)
    b.create_table :codes do |t|
      t.string    :name, :limit => 500, :null => false
      t.string    :short, :limit => 100
      t.string    :short_sort, :limit => 100
      t.datetime  :version, :null => false, :default => "2014-01-01"
      t.integer   :rev, :null => false, :default => 1
      t.integer   :size, :null => false, :default => 1
      t.integer   :domain_id, :null => false
    end
  end

  private

  #----------------------------------------------------------------------------
  def clear_linebreaks
    if name
      cstr = name.gsub(/\r?\n/, " ")
      self.name = cstr.strip
    end
  end

  #----------------------------------------------------------------------------
  def make_sortable_short
    if short
      self.short_sort = short.gsub(/Ä/, "a")
                             .gsub(/Ö/, "o")
                             .gsub(/Ü/, "u")
                             .downcase
                             .gsub(/ä/, "a")
                             .gsub(/ö/, "o")
                             .gsub(/ü/, "u")
                             .gsub(/ß/, "s")
    end
  end
end
