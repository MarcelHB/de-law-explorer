/* SPDX-License-Identifier: GPL-3.0 */

#include "exception_with_code.h"

ExceptionWithCode::ExceptionWithCode(int32_t code) :
  _code(code)
{}

const char* ExceptionWithCode::what() const throw() {
    return "Generic Exception";
}

int32_t ExceptionWithCode::code() const {
    return this->_code;
}
