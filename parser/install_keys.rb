# SPDX-License-Identifier: GPL-3.0
require File.join(File.dirname(__FILE__), "loader.rb")

Database.initialize_for("test")

key_param = ARGV[0]

case key_param
when "linker"
  ActiveRecord::Schema.define do |b|
    CodeAlias.add_table_keys(b)
    CodeAtom.add_table_keys(b)
    CodeAtomRelation.add_table_keys(b)
  end
when "post_linker"
  ActiveRecord::Schema.define do |b|
    CodeLink.add_table_keys(b)
  end
else
  Database.setup_keys
end
