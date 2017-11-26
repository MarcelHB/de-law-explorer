# SPDX-License-Identifier: GPL-3.0
class InterCodeLink < ActiveRecord::Base
  belongs_to  :code
  belongs_to  :to_code

  #----------------------------------------------------------------------------
  def self.create_table(b)
    b.create_table :inter_code_links do |t|
      t.integer   :code_id, :null => false
      t.integer   :to_code_id, :null => false
      t.integer   :count, :null => false, :default => 1
    end
  end

  #----------------------------------------------------------------------------
  def self.add_table_keys(b)
    b.add_index   :inter_code_links, [:code_id]
    b.add_index   :inter_code_links, [:to_code_id]
  end
end

