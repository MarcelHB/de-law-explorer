# SPDX-License-Identifier: GPL-3.0
$LOAD_PATH << File.dirname(__FILE__)

GEM_REQUIRES = %w(fileutils
                  nokogiri
                  open-uri
                  zip
                  net/http
                  active_record
                  sqlite3
                  yaml
                  polyglot
                  thread
                  treetop)

GEM_REQUIRES.each { |gem| require(gem) }

%w(utils models scanner restorer linker downloader).each do |subdir|
  Dir[ File.dirname(__FILE__) + "/#{subdir}/**/*.rb" ].each { |file| require(file) }
end
