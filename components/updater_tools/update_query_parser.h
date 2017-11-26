/* SPDX-License-Identifier: GPL-3.0 */

#ifndef UPDATE_QUERY_PARSER_H
#define UPDATE_QUERY_PARSER_H

#include <QByteArray>

struct UpdateQueryParserData {
   size_t major_version;
   size_t minor_version;
   uint32_t db_version;
   QString app_update_url;
   QString db_update_url;
};

class UpdateQueryParser {
  public:
    UpdateQueryParser(const QByteArray&);
    bool parse(UpdateQueryParserData&);
  private:
    const QByteArray& raw_data;
};

#endif // UPDATE_QUERY_PARSER_H
