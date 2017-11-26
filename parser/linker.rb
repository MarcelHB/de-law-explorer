# SPDX-License-Identifier: GPL-3.0
require File.join(File.dirname(__FILE__), "loader.rb")

# Otherwise, on Windows, Treetop fails working with CP850
Encoding.default_external = "UTF-8"

DEFAULT_DOMAIN = "BRD"
DEFAULT_LINKER = "de"

Database.initialize_for("test")

domain_param = DEFAULT_DOMAIN
linker_param = DEFAULT_LINKER
stop_on_error = false

if ARGV.length > 1
  ARGV.each_index do |i|
    if i > 0
      param = ARGV[i]
      if param[0..8] == "--domain="
        domain_param = param[9..-1]
      elsif param[0..8] == "--linker="
        scanner_param = param[9..-1]
      elsif param == "--stop-on-error"
        stop_on_error = true
      end
    end
  end
end

linker_class = case linker_param
               when "de"
                 Linker::De
               else
                 raise "Unknown linker!"
               end

domain = Domain.find_by_code(domain_param)

unless domain
  raise "Unknown domain!"
end

domain.codes.each do |code|
  code.code_atoms.where(:atom_type => CodeAtom::TYPE_SATZ).find_in_batches do |batch|
    batch.each do |phrase|
      puts "Scanning #{phrase.id}"
      begin
        linker_class.new(phrase).solve
      rescue => ex
        if stop_on_error
          raise ex
        else
          $stderr.puts("ERROR in #{phrase.id}: #{ex.message}")
          $stderr.puts(ex.backtrace.join("\n"))
        end
      end
    end
  end
end
