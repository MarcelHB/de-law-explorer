/* SPDX-License-Identifier: GPL-3.0 */

#ifndef CODE_ATOM_TREE_REVERSE_PATH_BY_IDS_LOADER_H
#define CODE_ATOM_TREE_REVERSE_PATH_BY_IDS_LOADER_H

#include <QVector>

#include "code_atom_tree_path_loader.h"

class CodeAtomTreeReversePathByIDsLoader : public DBLoader<std::vector<CodeAtom>> {
  public:
    CodeAtomTreeReversePathByIDsLoader(QVector<uint32_t>, size_t);
    bool next_row(std::vector<CodeAtom>&);
  protected:
    QSqlQuery query();
  private:
    QVector<uint32_t> ids;
    size_t depth;

    QString build_query();
};

#endif // CODE_ATOM_TREE_REVERSE_PATH_BY_IDS_LOADER_H
