# SPDX-License-Identifier: GPL-3.0
class Domain < ActiveRecord::Base
  has_many  :codes

  ASSETS_FILE = File.join(File.dirname(__FILE__), "domains.yml")

  #----------------------------------------------------------------------------
  def self.create_table(b)
    b.create_table :domains do |t|
      t.string    :name, :limit => 500, :null => false
      t.string    :code, :limit => 10, :null => false
    end

    create_initial_domains
  end

  #----------------------------------------------------------------------------
  def self.create_initial_domains
    yaml = YAML.load_file(ASSETS_FILE)

    yaml.each_pair do |k,v|
      create({
        :name => v,
        :code => k
      })
    end
  end
end
