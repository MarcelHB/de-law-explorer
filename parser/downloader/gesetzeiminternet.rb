# SPDX-License-Identifier: GPL-3.0
module Downloader
  class GesetzeImInternet
    URL = "https://www.gesetze-im-internet.de"

    #--------------------------------------------------------------------------
    def initialize(folder)
      self.folder = folder
    end

    #--------------------------------------------------------------------------
    def action
      doc = Nokogiri::HTML(open("#{URL}/aktuell.html"))
      doc.css("a[class~=alphabet]").map do |link|
        parse_list(link['href'])
      end
    end

    #--------------------------------------------------------------------------
    def parse_list(link)
      trimmed_link = link[1..-1]

      puts "Processing: " + URL + trimmed_link

      doc = Nokogiri::HTML(open(URL + trimmed_link))
      doc.css('div#paddingLR12 p').map do |p|
        download_file(p.children.first['href'])
      end
    end

    #--------------------------------------------------------------------------
    def download_file(link)
      short = link[2..-12]
      url = link[1..-11] + "xml.zip"
      host = URL[7..-1]
      FileUtils.mkdir_p(File.join(folder, short))

      puts "Downloading: #{url} to #{short}"

      Net::HTTP.start(host) do |http|
        resp = http.get(url)

        case resp
        when Net::HTTPSuccess then
          tf = Tempfile.new(short)
          tf.binmode
          begin
            tf.write(resp.body)
            tf.close
            Zip::File.open(tf.path) do |zf|
              zf.each do |item|
                out_file = File.join(folder, short, item.name)
                File.exists?(out_file) ? File.unlink(out_file) : nil
                item.extract(out_file)
              end
            end
          rescue NoMethodError
            # Sometimes, the zip lib raises an error
          ensure
            tf.close
            tf.unlink
          end
        end
      end
    end

    private
    attr_accessor :folder
  end
end
