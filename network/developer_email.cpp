/* SPDX-License-Identifier: GPL-3.0 */

#include "developer_email.h"

DeveloperEMail::DeveloperEMail(DeveloperMailType t, const QString &custom,
                               const QString &system) :
  type(t),
  custom_message(custom),
  system_message(system)
{}

QString DeveloperEMail::body() const {
    QString body;

    body.append("SYSTEM:\n\n");
    body.append(this->system_message);
    body.append("\n\n");
    body.append("========:\n\n");
    body.append(this->custom_message);

    return body;
}

QString DeveloperEMail::receiver() const {
    return QString("<the_receiver>");
}

QString DeveloperEMail::sender() const {
    return QString("<some_untrusted_sender>");
}

QString DeveloperEMail::subject() const {
    switch(this->type) {
        case DeveloperEMail::TYPE_FEEDBACK:
            return QString("[de-law-expl][FEEDBACK]");
        case DeveloperEMail::TYPE_EXCEPTION:
            return QString("[de-law-expl][EXCEPTION]");
        case DeveloperEMail::TYPE_ERROR:
            return QString("[de-law-expl][USER ERROR]");
        default:
            return QString("[de-law-expl][FUCKED UP]");
    }
}
