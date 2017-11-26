/* SPDX-License-Identifier: GPL-3.0 */

#include <QNetworkAccessManager>
#include <QNetworkRequest>

#include "update_query.h"

UpdateQuery::UpdateQuery(size_t major, size_t minor, uint32_t db) :
  version_major(major),
  version_minor(minor),
  db_version(db),
  reply(nullptr),
  done(false)
{}

void UpdateQuery::run() {
    QNetworkAccessManager manager;
    QNetworkRequest request(QUrl(QString(UPDATE_INFO_URL)));

    connect(&manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(on_response_received(QNetworkReply*)));

    this->reply = manager.get(request);

    connect(this->reply, SIGNAL(finished()),
            this, SLOT(quit()));

    this->exec();
}

void UpdateQuery::on_response_received(QNetworkReply *response) {
    if(response->error() == QNetworkReply::NoError) {
        QByteArray data = response->peek(response->bytesAvailable());
        UpdateQueryParser parser(data);
        UpdateQueryParserData parser_data;

        if(parser.parse(parser_data)) {
            UpdateQueryData update_data;
            this->evaluate_query_data(parser_data, update_data);
            emit this->update_data_received(update_data);
        } else {
            emit this->update_impossible(PARSER_ERROR);
        }
    } else {
        UpdateQueryError error = OTHER_ERROR;

        switch(response->error()) {
          case QNetworkReply::ConnectionRefusedError:
          case QNetworkReply::RemoteHostClosedError:
          case QNetworkReply::HostNotFoundError:
            error = NETWORK_ERROR;
            break;
          case QNetworkReply::ContentAccessDenied:
          case QNetworkReply::ContentNotFoundError:
            error = OBJECT_ERROR;
          default:
            break;
        }

        emit this->update_impossible(error);
    }

    response->deleteLater();
    this->done = true;
    this->quit();
}

void UpdateQuery::evaluate_query_data(UpdateQueryParserData &remote_data,
                                      UpdateQueryData &data) {
    data.version_major = remote_data.major_version;
    data.version_minor = remote_data.minor_version;
    data.database_version = remote_data.db_version;
    data.application_update_url = remote_data.app_update_url;
    data.database_update_url = remote_data.db_update_url;
    data.has_application_update = data.has_database_update = false;

    if(this->version_major < remote_data.major_version ||
       this->version_minor < remote_data.minor_version) {
        data.has_application_update = true;
    }

    if(this->db_version < remote_data.db_version) {
        data.has_database_update = true;
    }
}

void UpdateQuery::abort() {
    if(this->reply != nullptr && !this->done) {
        this->reply->abort();
        this->quit();
    }
}
