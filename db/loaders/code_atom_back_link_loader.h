/* SPDX-License-Identifier: GPL-3.0 */

#ifndef CODE_ATOM_BACK_LINK_LOADER_H
#define CODE_ATOM_BACK_LINK_LOADER_H

#include <cstdint>
#include <sstream>

#include "loader.h"
#include "../orm_structs/code_atom_link.h"

using namespace std;

class CodeAtomBackLinkLoader : public DBLoader<CodeAtomLink> {
public:
    CodeAtomBackLinkLoader(uint32_t _id);
    bool next_row(CodeAtomLink &codeAtomLink);
protected:
    QSqlQuery query();
private:
    uint32_t id;
};

#endif // CODE_ATOM_BACK_LINK_LOADER_H
