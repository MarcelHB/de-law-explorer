# SPDX-License-Identifier: GPL-3.0
module AtomTokenizer
  def atom_token(type, key)
    case type
    when CodeAtom::TYPE_PARAGRAPH
      "§ #{key}"
    when CodeAtom::TYPE_ARTIKEL
      "Art. #{key}"
    when CodeAtom::TYPE_ABSATZ
      "Abs. #{key}"
    when CodeAtom::TYPE_SATZ
      "Satz #{key}"
    when CodeAtom::TYPE_NUMMER
      "Nr. #{key}"
    when CodeAtom::TYPE_ANHANG
      "Anlage #{key}"
    else
      nil
    end
  end
end
