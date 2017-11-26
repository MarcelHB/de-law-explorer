/* SPDX-License-Identifier: GPL-3.0 */

#ifndef CODE_ATOM_MAX_DEPTH_LOADER_H
#define CODE_ATOM_MAX_DEPTH_LOADER_H

#include <cstdint>

#include "loader.h"

class CodeAtomMaxDepthLoader : public DBLoader<uint32_t> {
  public:
    CodeAtomMaxDepthLoader(QVector<uint32_t>&);
    bool next_row(uint32_t&);
  protected:
    QSqlQuery query();
  private:
    QVector<uint32_t> ids;

    void ids_to_stringlist(QStringList&);
};

#endif // CODE_ATOM_MAX_DEPTH_LOADER_H
