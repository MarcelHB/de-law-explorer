/* SPDX-License-Identifier: GPL-3.0 */

#include "controller_response.h"

ControllerResponse::ControllerResponse() :
    ControllerResponse(QString(""))
{}

ControllerResponse::ControllerResponse(const QString &req_id) :
  _request_id(req_id),
  _return_code(0),
  _response(QString(""))
{}

void ControllerResponse::succesful_response(const QString &r) {
    this->_response = QString(r);
}

void ControllerResponse::bad_response(const QString &r, int code = 1) {
    this->_response = QString(r);
    this->_return_code = code;
}

const QString& ControllerResponse::request_id() const {
    return this->_request_id;
}

const QString& ControllerResponse::response() const {
    return this->_response;
}

int ControllerResponse::return_code() const {
    return this->_return_code;
}

bool ControllerResponse::was_successful() const {
    return this->_return_code == 0;
}
