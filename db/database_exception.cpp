/* SPDX-License-Identifier: GPL-3.0 */

#include "database_exception.h"

#define DB_EXCEPTION_CODE 47474747

DatabaseException::DatabaseException(QString &str) :
    ExceptionWithCode(DB_EXCEPTION_CODE),
    error_str(str)
{}

const char* DatabaseException::what() const throw() {
    QByteArray chars = error_str.toLocal8Bit();
    return chars.data();
}
