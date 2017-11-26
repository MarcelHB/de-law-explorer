/* SPDX-License-Identifier: GPL-3.0 */

#ifndef DE_LAW_EXPLORER_DB_LOADER_H
#define DE_LAW_EXPLORER_DB_LOADER_H

#include <QtSql>

#include "loader_base.h"
#include "../database_exception.h"

/* A generic QSqlDatabase driven SELECT-loader to write to a T on every
 *  next_row(). */

template <class T>
class DBLoader : public LoaderBase {
  public:
    virtual ~DBLoader() {}
    void load();
    virtual bool next_row(T&) = 0;
  protected:
    virtual QSqlQuery query() = 0;
    QSqlQuery& last_query();

  private:
    QSqlQuery _last_query;
};

template <class T>
void DBLoader<T>::load() {
    this->_last_query = this->query();
    if(!this->_last_query.exec()) {
        QSqlError error = this->_last_query.lastError();
        QString err_str = error.text();
        throw new DatabaseException(err_str);
    }
}

template <class T>
QSqlQuery& DBLoader<T>::last_query() {
    return this->_last_query;
}

#endif
