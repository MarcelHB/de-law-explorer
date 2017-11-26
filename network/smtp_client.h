/* SPDX-License-Identifier: GPL-3.0 */

#ifndef SMTP_CLIENT_H
#define SMTP_CLIENT_H

#include <QMetaType>
#include <QSslSocket>
#include <QTextStream>

#include "developer_email.h"

class SMTPClient : public QObject {
  Q_OBJECT

  public:
    enum SMTPClientState {
        INIT, AUTH, AUTH_NAME, AUTH_PW,
        FROM, RECV, DATA, BODY, QUIT, HALT
    };
    SMTPClient(DeveloperEMail&);
    ~SMTPClient();

    void send();
  signals:
    void step(SMTPClient::SMTPClientState);
    void error(SMTPClient::SMTPClientState);
    void success();
    void socket_closed();
  private slots:
    void on_socket_ready_read();
    void on_socket_disconnected();
    void on_socket_error(QAbstractSocket::SocketError);
    void on_ssl_errors(QList<QSslError>);
    void on_ssl_peer_error(QSslError);
  private:
    QSslSocket *socket;
    QTextStream *sstream;
    DeveloperEMail& email;
    SMTPClientState state;

    void initialize_socket();
    void close();
    void error_close();
    QString quoted_printable(const QByteArray&);
};

Q_DECLARE_METATYPE(SMTPClient::SMTPClientState)

#endif // SMTP_CLIENT_H
