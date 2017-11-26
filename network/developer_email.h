/* SPDX-License-Identifier: GPL-3.0 */

#ifndef DE_LAW_EXPLORER_DEVELOPER_EMAIL_H
#define DE_LAW_EXPLORER_DEVELOPER_EMAIL_H

#include <QMetaType>
#include <QString>

class DeveloperEMail {
  public:
    enum DeveloperMailType { TYPE_FEEDBACK, TYPE_EXCEPTION, TYPE_ERROR };
    DeveloperEMail(DeveloperMailType, const QString&, const QString&);

    QString body() const;
    QString receiver() const;
    QString sender() const;
    QString subject() const;

  private:
    DeveloperMailType type;
    const QString custom_message;
    const QString system_message;
};

Q_DECLARE_METATYPE(DeveloperEMail::DeveloperMailType)

#endif
