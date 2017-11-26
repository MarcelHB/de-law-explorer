/* SPDX-License-Identifier: GPL-3.0 */

#ifndef LOADER_BASE_H
#define LOADER_BASE_H

#include <QtSql>

class LoaderBase {
  public:
    static void default_db(QSqlDatabase&);
  protected:
    static QSqlDatabase* default_db();
  private:
    /* Use this to offer the default database. */
    static QSqlDatabase *_default_db;
};

#endif // LOADER_BASE_H
