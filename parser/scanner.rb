# SPDX-License-Identifier: GPL-3.0
require File.join(File.dirname(__FILE__), "loader.rb")

DEFAULT_DOMAIN = "BRD"
DEFAULT_SCANNER = "juris_de"

if ARGV.length < 1
  puts "\nScanner front-end, use like:"
  puts "\t bundle exec ruby scanner.rb <file> [<domain>] [<scanner>]"
  exit
end

Database.initialize_for("test")

file_to_scan = ARGV[0]
domain_param = ARGV[1] || DEFAULT_DOMAIN
scanner_param = ARGV[2] || DEFAULT_SCANNER

scanner = nil

scanner = case scanner_param
          when "juris_de"
            Scanner::JurisDE.new(file_to_scan)
          else
            raise "Unknown scanner!"
          end

domain = Domain.find_by_code(domain_param)

unless domain
  raise "Unknown domain!"
end

scanner.action(domain)
