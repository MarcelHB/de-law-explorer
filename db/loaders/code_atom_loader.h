/* SPDX-License-Identifier: GPL-3.0 */

#ifndef CODE_ATOM_LOADER_H
#define CODE_ATOM_LOADER_H

#include <cstdint>
#include <sstream>

#include "loader.h"
#include "../orm_structs/code_atom.h"

using namespace std;

class CodeAtomLoader : public DBLoader<CodeAtom> {
public:
    CodeAtomLoader(uint32_t);
    bool next_row(CodeAtom&);
protected:
    QSqlQuery query();
private:
    uint32_t id;
};

#endif // CODE_ATOM_LOADER_H
