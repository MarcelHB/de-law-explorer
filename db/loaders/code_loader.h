/* SPDX-License-Identifier: GPL-3.0 */

#ifndef DE_LAW_EXPLORER_CODE_ATOM_LOADER_H
#define DE_LAW_EXPLORER_CODE_ATOM_LOADER_H

#include <cstdint>

#include "loader.h"
#include "../orm_structs/code.h"

class CodeLoader : public DBLoader<Code> {
  public:
    CodeLoader(uint32_t);
    bool next_row(Code&);
  protected:
    QSqlQuery query();
  private:
    uint32_t id;
};

#endif
