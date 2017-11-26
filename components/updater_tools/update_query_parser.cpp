/* SPDX-License-Identifier: GPL-3.0 */

#include <QString>
#include <QStringList>

#include "update_query_parser.h"

UpdateQueryParser::UpdateQueryParser(const QByteArray &bytes) :
  raw_data(bytes)
{}

bool UpdateQueryParser::parse(UpdateQueryParserData &data) {
    QString body(this->raw_data);
    QStringList lines = body.split('\n');

    if(lines.length() != 2) {
        return false;
    }

    const QString& app_line = lines.at(0);
    QStringList tokens = app_line.split(' ');

    if(tokens.length() != 3) {
        return false;
    }

    data.major_version = tokens[0].toUInt();
    data.minor_version = tokens[1].toUInt();
    data.app_update_url = tokens[2];

    const QString& db_line = lines.at(1);
    tokens = db_line.split(' ');

    if(tokens.length() != 2) {
        return false;
    }

    data.db_version = tokens[0].toUInt();
    data.db_update_url = tokens[1];

    return true;
}
