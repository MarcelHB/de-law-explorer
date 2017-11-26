/* SPDX-License-Identifier: GPL-3.0 */

#ifndef DATABASE_H
#define DATABASE_H

#include <cstdint>

#include <QtSql>

class Database {
  public:
    Database(const QString&);
    ~Database();

    QString file_name() const;
    QSqlDatabase& handle();
    bool open();
    QString open_error() const;
    uint32_t version();
  private:
    QSqlDatabase *_handle;
    QString _file_name;
    QString error_string;
};

#endif // DATABASE_H
