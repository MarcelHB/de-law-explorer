# SPDX-License-Identifier: GPL-3.0
require File.join(File.dirname(__FILE__), "loader.rb")

DEFAULT_RESTORER = "html"

if ARGV.length < 1
  puts "\nParser restoration, use like:"
  puts "\t bundle exec ruby restore.rb <codeID> [<path>] [<restorer>]"
  exit
end

Database.initialize_for("test")

code_param = ARGV[0].to_i
path_param = ARGV[1]
restore_param = ARGV[2] || DEFAULT_RESTORER

code = Code.find(code_param)

path = if path_param && path_param != ""
         path_param
       else
         File.join(File.dirname(__FILE__), "output", "#{code.short}.html")
       end

unless code
  raise "No code found for ID #{code_param}"
end

restorer = case restore_param
           when "html"
             Restorer::HTML.new(code)
           else
             raise "Unknown restorer!"
           end

FileUtils.mkdir_p(File.dirname(path))
restorer.action(path)
