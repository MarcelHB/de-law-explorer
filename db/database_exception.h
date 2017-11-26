/* SPDX-License-Identifier: GPL-3.0 */

#ifndef DE_LAW_EXPLORER_DB_EXCEPTION_H
#define DE_LAW_EXPLORER_DB_EXCEPTION_H

#include <string>
#include <QString>

#include "../exception_with_code.h"

class DatabaseException : public ExceptionWithCode {
  public:
    DatabaseException(QString&);
    const char* what() const throw();
  private:
    QString error_str;
};

#endif
