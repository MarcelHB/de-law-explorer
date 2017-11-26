# SPDX-License-Identifier: GPL-3.0

module PhraseMaskerDE
  #--------------------------------------------------------------------------
  def mask_nonphrase_stops(text)
    text = sanitize_static_abbrevs(text)
    sanitize_numerals(text)
  end

  private

  # constant terms that should not interfere with anything else
  #--------------------------------------------------------------------------
  def sanitize_static_abbrevs(text)
    text.gsub(/S\./, "S*")
      .gsub(/Bl\./, "Bl*")
      .gsub(/Bek\./, "Bek*")
      .gsub(/Nr\./, "Nr*")
      .gsub(/Abs\./, "Abs*")
      .gsub(/Art\./, "Art*")
      .gsub(/(\s+)v\./, '\1v*')
      .gsub(/(\s+)d\./, '\1d*')
      .gsub(/e\.\s*V\./, "e*V*")
      .gsub(/gem\./, "gem*")
      .gsub(/z\.\s*B\./, "z*B*")
      .gsub(/l\.r\./, "l*r*")
      .gsub(/v\.H\./, "v*H*")
      .gsub(/bzgl\./, "bzgl*")
      .gsub(/vgl\./, "vgl*")
      .gsub(/Lfd\./, "Lfd*")
      .gsub(/Bundesgesetzbl./, "Bundesgesetzbl*")
  end

  # dates, number seperators ...
  #--------------------------------------------------------------------------
  def sanitize_numerals(text)
    # DD.MM.YYYY
    text.gsub(/(\D*\d{1,2})\.(\d{1,2})\.(\d{4}\D*)/, '\1*\2*\3')
      .gsub(/(\d)\.(\d{3})/, '\1*\2') # 10.000.000 etc.
      .gsub(/(\d{1,2})\.(\s+(?:Januar|Februar|März|April|Mai|Juni|Juli|August|September|Oktober|November|Dezember))/, '\1*\2')
      .gsub(/(\d{1,3})\.(\s+Lebensjahr)/, '\1*\2')
      .gsub(/(\s+(?:der|die|das|den|dem|des)\s+\d+)\.([\s\w,])/, '\1*\2')
      .gsub(/(\d{1,2})\.(\s+(?:Tag|Monat|Jahr))/, '\1*\2') # 10. Tag
      .gsub(/\.(\d+)/, '*\1')
  end
end
