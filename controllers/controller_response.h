/* SPDX-License-Identifier: GPL-3.0 */

#ifndef CONTROLLER_RESPONSE_H
#define CONTROLLER_RESPONSE_H

#include <cstdint>
#include <QString>

class ControllerResponse {
  public:
    ControllerResponse();
    ControllerResponse(const QString&);
    void succesful_response(const QString&);
    void bad_response(const QString&, int code);

    const QString& request_id() const;
    int return_code() const;
    const QString& response() const;

    bool was_successful() const;
  private:
    const QString _request_id;
    int _return_code;
    QString _response;
};

#endif // CONTROLLER_RESPONSE_H
