/* SPDX-License-Identifier: GPL-3.0 */

#ifndef CODE_ATOM_TREE_PATH_REVERSE_LOADER_H
#define CODE_ATOM_TREE_PATH_REVERSE_LOADER_H

#include <sstream>
#include <vector>

#include "loader.h"
#include "../orm_structs/code_atom.h"

class CodeAtomTreePathReverseLoader : public DBLoader<std::vector<CodeAtom>> {
    public:
      CodeAtomTreePathReverseLoader(uint32_t, size_t);
      bool next_row(std::vector<CodeAtom> &list);
    protected:
      QSqlQuery query();
    private:
      uint32_t id;
      size_t depth;

      QString build_query();
      bool row_cols_to_code_atom(QSqlQuery&, CodeAtom&, uint32_t);
};

#endif // CODE_ATOM_TREE_PATH_REVERSE_LOADER_H
