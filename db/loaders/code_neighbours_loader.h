/* SPDX-License-Identifier: GPL-3.0 */

#ifndef CODE_ATOM_NEIGHBOURS_LOADER_H
#define CODE_ATOM_NEIGHBOURS_LOADER_H

#include <cstdint>

#include "loader.h"
#include "../orm_structs/code_neighbour.h"

class CodeNeighboursLoader : public DBLoader<CodeNeighbour> {
  public:
    CodeNeighboursLoader(uint32_t);
    bool next_row(CodeNeighbour&);
  protected:
    QSqlQuery query();
  private:
    uint32_t id;
};

#endif
