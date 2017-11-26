/* SPDX-License-Identifier: GPL-3.0 */

#ifndef CODE_ATOM_FINDER_H
#define CODE_ATOM_FINDER_H

#include <cstdint>

#include "loader.h"
#include "../orm_structs/code_atom.h"
#include "../atom_type_enum.h"

class CodeAtomFinder : public DBLoader<CodeAtom> {
  public:
    CodeAtomFinder(const QString&, uint32_t, CodeAtomType type = TYPE_PARAGRAPH);
    bool next_row(CodeAtom&);
  protected:
    QSqlQuery query();
  private:
    const QString& pattern;
    uint32_t code_id;
    CodeAtomType type;
};

#endif // CODE_ATOM_FINDER_H
