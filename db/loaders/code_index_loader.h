/* SPDX-License-Identifier: GPL-3.0 */

#ifndef CODE_INDEX_LOADER_H
#define CODE_INDEX_LOADER_H

#include <cstdint>

#include "loader.h"
#include "../orm_structs/code.h"

class CodeIndexLoader : public DBLoader<Code> {
  public:
    CodeIndexLoader(const QChar);
    bool next_row(Code&);
  protected:
    QSqlQuery query();

  private:
    QChar c;
};

#endif
