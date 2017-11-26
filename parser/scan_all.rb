# SPDX-License-Identifier: GPL-3.0
require File.join(File.dirname(__FILE__), "loader.rb")

DEFAULT_DOMAIN = "BRD"
DEFAULT_SCANNER = "juris_de"

if ARGV.length < 1
  puts "\nScanner front-end for mass use:"
  puts "\t bundle exec ruby scan_all.rb #<Dir::glob pattern># [OPTIONS]"
  puts "\n\tOPTIONS:"
  puts "\t\t--domain=<domain>"
  puts "\t\t--scanner=<scanner>"
  puts "\t\t--stop-on-error"
end

Database.initialize_for("test")

pattern = ARGV[0][1..-2]
domain_param = DEFAULT_DOMAIN
scanner_param = DEFAULT_SCANNER
stop_on_error = false

if ARGV.length > 1
  ARGV.each_index do |i|
    if i > 0
      param = ARGV[i]
      if param[0..8] == "--domain="
        domain_param = param[9..-1]
      elsif param[0..9] == "--scanner="
        scanner_param = param[10..-1]
      elsif param == "--stop-on-error"
        stop_on_error = true
      end
    end
  end
end

domain = Domain.find_by_code(domain_param)

unless domain
  raise "Unknown domain!"
end

scanner_class = case scanner_param
                when "juris_de"
                  Scanner::JurisDE
                else
                  raise "Unknown scanner!"
                end

Dir.glob(pattern).sort.each do |path|
  begin
    puts "Scanning #{path}"
    scanner = scanner_class.new(path)
    scanner.action(domain)
  rescue => ex
    if stop_on_error
      raise ex
    else
      $stderr.puts("ERROR in #{path}: #{ex.message}")
      $stderr.puts(ex.backtrace.join("\n"))
    end
  end
end
