/* SPDX-License-Identifier: GPL-3.0 */

#include "database.h"

Database::Database(const QString &file_name) :
  _handle(nullptr),
  _file_name(file_name)
{}

Database::~Database() {
    if(this->_handle != nullptr) {
        this->_handle->close();
        delete this->_handle;
    }
}

QString Database::file_name() const {
    return this->_file_name;
}

QSqlDatabase& Database::handle() {
    return *this->_handle;
}

bool Database::open() {
    this->_handle = new QSqlDatabase();
    *this->_handle = QSqlDatabase::addDatabase("QSQLITE");
    this->_handle->setDatabaseName(this->_file_name);

    bool result = this->_handle->open();
    if(!result) {
        this->error_string = this->_handle->lastError().text();
    }

    return result;
}

QString Database::open_error() const {
    return this->error_string;
}

uint32_t Database::version() {
    QSqlQuery query(*this->_handle);
    uint32_t version = 0;

    query.exec("PRAGMA user_version");
    if(query.next()) {
        version = query.value(0).toUInt();
    }

    return version;
}
