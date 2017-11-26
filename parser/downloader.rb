# SPDX-License-Identifier: GPL-3.0
require File.join(File.dirname(__FILE__), "loader.rb")

DEFAULT_SOURCE = "gesetze_im_internet"
DEFAULT_FOLDER = "xml"

source = ARGV[0] || DEFAULT_SOURCE
folder = ARGV[1] || File.join(File.dirname(__FILE__), DEFAULT_FOLDER)

downloader = case source
          when "gesetze_im_internet"
            Downloader::GesetzeImInternet.new(folder)
          else
            raise "Unknown source!"
          end

downloader.action
