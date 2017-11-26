/* SPDX-License-Identifier: GPL-3.0 */

#include <QDebug>
#include <QDateTime>
#include <QThread>

#include "smtp_client.h"

/**
 * Set an auth name and password to some underprivileged smtp
 * account, only being capable to send to one single address!
 */

SMTPClient::SMTPClient(DeveloperEMail &_email) :
  socket(nullptr),
  sstream(nullptr),
  email(_email),
  state(INIT)
{}

SMTPClient::~SMTPClient() {
    this->close();
}

void SMTPClient::send() {
    if(this->socket == nullptr) {
        this->state = INIT;
        this->initialize_socket();
    }
}

void SMTPClient::initialize_socket() {
    this->socket = new QSslSocket;
    emit this->step(this->state);

    connect(this->socket, SIGNAL(readyRead()),
            this, SLOT(on_socket_ready_read()));
    connect(this->socket, SIGNAL(sslErrors(const QList<QSslError>)),
            this, SLOT(on_ssl_errors(QList<QSslError>)));
    connect(this->socket, SIGNAL(peerVerifyError(QSslError)),
            this, SLOT(on_ssl_peer_error(QSslError)));
    connect(this->socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(on_socket_error(QAbstractSocket::SocketError)));
    connect(this->socket, SIGNAL(disconnected()),
            this, SLOT(on_socket_disconnected()));

    this->socket->connectToHostEncrypted("", 465); /* Host. */
    this->sstream = new QTextStream(this->socket);
}

void SMTPClient::on_ssl_peer_error(QSslError err) {
    qDebug() << "SSL PEER ERROR";
    qDebug() << err.errorString();
    if(this->state != HALT) {
        this->error_close();
    }
}

void SMTPClient::on_socket_disconnected() {
    emit this->socket_closed();
}

void SMTPClient::on_socket_error(QAbstractSocket::SocketError) {
    qDebug() << "SOCKET ERROR";
    qDebug() << this->socket->errorString();
    if(this->state != HALT) {
        this->error_close();
    }
}

void SMTPClient::on_socket_ready_read() {
    QString response;
    do {
        response = socket->readLine();
    } while (socket->canReadLine() && response[3] != ' ');

    response.truncate(3);

    if(this->state == INIT && response[0] == '2') {
        *(this->sstream) << "EHLO \r\n"; /* HERE. */
        this->sstream->flush();
        this->state = AUTH;
    } else if(this->state == AUTH && response[0] == '2') {
        *(this->sstream) << "AUTH LOGIN\r\n";
        this->sstream->flush();
        this->state = AUTH_NAME;
    } else if(this->state == AUTH_NAME && response[0] == '3') {
        *(this->sstream) << ""; /* HERE. */
        this->sstream->flush();
        this->state = AUTH_PW;
    } else if(this->state == AUTH_PW && response[0] == '3') {
        *(this->sstream) << ""; /* HERE. */
        this->sstream->flush();
        this->state = FROM;
    } else if(this->state == FROM && response[0] == '2') {
        *(this->sstream) << "MAIL FROM:" << this->email.sender() << "\r\n";
        this->sstream->flush();
        this->state = RECV;
    } else if(this->state == RECV && response[0] == '2') {
        *(this->sstream) << "RCPT TO:" << this->email.receiver() << "\r\n";
        this->sstream->flush();
        this->state = DATA;
    } else if(this->state == DATA && response[0] == '2') {
        *(this->sstream) << "DATA\r\n";
        this->sstream->flush();
        this->state = BODY;
    } else if(this->state == BODY && response[0] == '3') {
        QDateTime now = QDateTime::currentDateTime();
        QString body = this->quoted_printable(this->email.body().toUtf8());

        *(this->sstream) << "From: " << this->email.sender() << "\r\n"
            << "To: " << this->email.receiver() << "\r\n"
            << "Subject: " << this->email.subject() << "\r\n"
            << "Date: " << now.toString("yyyy-MM-dd HH:mm:ss") << "\r\n"
            << "MIME-Version: 1.0\r\n"
            << "Content-Type: text/plain; charset=UTF-8\r\n"
            << "Content-Transfer-Encoding: quoted-printable\r\n\r\n"
            << body << "\r\n.\r\n";
        this->sstream->flush();
        this->state = QUIT;
    } else if(this->state == QUIT && response[0] == '2') {
        *(this->sstream) << "QUIT\r\n";
        this->sstream->flush();
        this->state = HALT;
        emit this->success();
        return;
    } else if(this->state == HALT) {
        this->close();
        return;
    } else {
        this->error_close();
        return;
    }

    emit this->step(this->state);
}

void SMTPClient::on_ssl_errors(QList<QSslError> errors) {
    qDebug() << "SSL ERRORS";
    for(auto it = errors.cbegin(); it != errors.cend(); ++it) {
        const QSslError& err = *it;
        qDebug() << err.errorString();
    }
    if(this->state != HALT) {
        this->error_close();
    }
}

void SMTPClient::error_close() {
    emit this->error(this->state);
    this->state = HALT;
}

void SMTPClient::close() {
    if(this->socket != nullptr) {
        this->socket->disconnectFromHost();
        delete this->socket;
        this->socket = nullptr;

        delete this->sstream;
        this->sstream = nullptr;
    }
}

QString SMTPClient::quoted_printable(const QByteArray &bytes) {
    QString str;
    size_t chars = 0;
    const char hex[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

    for(int i = 0; i < bytes.length(); ++i) {
        char byte = bytes.at(i);

        if(byte == 9 || (byte >= 32 && byte <= 60) || (byte >= 62 && byte <= 126)) {
            str.append(byte);
            chars++;
        } else {
            str.append("=");
            str.append(hex[(byte >> 4) & 0xF]);
            str.append(hex[byte & 0xF]);
            chars += 3;
        }

        if(chars >= 75) {
            str.append("=\r\n");
            chars = 0;
        }
    }

    return str;
}
