/* SPDX-License-Identifier: GPL-3.0 */

#ifndef CODE_ATOM_ROOT_LOADER_H
#define CODE_ATOM_ROOT_LOADER_H

#include <cstdint>
#include <sstream>

#include "loader.h"
#include "../orm_structs/code_atom.h"

using namespace std;

class CodeAtomRootLoader : public DBLoader<CodeAtom> {
public:
    CodeAtomRootLoader(uint32_t _codeId);
    bool next_row(CodeAtom&);
protected:
    QSqlQuery query();
private:
    uint32_t codeId;
};

#endif // CODE_ATOM_ROOT_LOADER_H
