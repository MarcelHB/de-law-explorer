/* SPDX-License-Identifier: GPL-3.0 */

#ifndef INTER_LINKS_LOADER_H
#define INTER_LINKS_LOADER_H

#include "loader.h"
#include "../orm_structs/code_atom_link.h"

class InterLinksLoader : public DBLoader<CodeAtomLink> {
  public:
    InterLinksLoader(uint32_t, uint32_t);
    bool next_row(CodeAtomLink&);
  protected:
    QSqlQuery query();
  private:
    uint32_t from_id;
    uint32_t to_id;
};

#endif // INTER_LINKS_LOADER_H
