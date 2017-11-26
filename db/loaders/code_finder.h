/* SPDX-License-Identifier: GPL-3.0 */

#ifndef CODE_FINDER_H
#define CODE_FINDER_H

#include <cstdint>

#include "loader.h"
#include "../orm_structs/code.h"

class CodeFinder : public DBLoader<Code> {
  public:
    CodeFinder(const QString&);
    bool next_row(Code&);
  protected:
    QSqlQuery query();
  private:
    const QString& pattern;
};

#endif // CODE_FINDER_H
