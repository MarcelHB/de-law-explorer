/* SPDX-License-Identifier: GPL-3.0 */

#ifndef DE_LAW_EXPLORER_EXCEPTION_WITH_CODE_H
#define DE_LAW_EXPLORER_EXCEPTION_WITH_CODE_H

#include <exception>
#include <cstdint>

class ExceptionWithCode : public std::exception {
  public:
    ExceptionWithCode(int32_t);
    virtual const char* what() const throw();
    int32_t code() const;
  private:
    int32_t _code;
};

#endif
