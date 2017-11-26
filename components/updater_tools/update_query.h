/* SPDX-License-Identifier: GPL-3.0 */

#ifndef DE_LAW_EXPLORER_UPDATE_QUERY_H
#define DE_LAW_EXPLORER_UPDATE_QUERY_H

#include <cstdint>

#include <QObject>
#include <QString>
#include <QThread>

#include <QNetworkReply>

#include "update_query_parser.h"

#define UPDATE_INFO_URL ""

enum UpdateQueryError { NETWORK_ERROR, OBJECT_ERROR, PARSER_ERROR, OTHER_ERROR };

struct UpdateQueryData {
    bool has_application_update;
    QString application_update_url;
    size_t version_major;
    size_t version_minor;

    bool has_database_update;
    QString database_update_url;
    uint32_t database_version;
};

class UpdateQuery : public QThread {
  Q_OBJECT
  public:
    UpdateQuery(size_t, size_t, uint32_t);
    void abort();

  signals:
    void update_impossible(UpdateQueryError);
    void update_data_received(const UpdateQueryData&);

  protected:
    void run();

  private slots:
    void on_response_received(QNetworkReply*);

  private:
    size_t version_major;
    size_t version_minor;
    uint32_t db_version;
    QNetworkReply *reply;
    bool done;

    void evaluate_query_data(UpdateQueryParserData&, UpdateQueryData&);
};

#endif
