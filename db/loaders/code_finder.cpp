/* SPDX-License-Identifier: GPL-3.0 */

#include "code_finder.h"

CodeFinder::CodeFinder(const QString &str) :
  pattern(str)
{}

QSqlQuery CodeFinder::query() {
    QSqlDatabase *db = default_db();
    QSqlQuery query(*db);

    QString like(this->pattern);
    like.append("%");

    query.prepare("SELECT c.id, c.name, c.short, c.size "
                  "FROM code_aliases ca "
                  "INNER JOIN codes c ON (c.id = ca.code_id) "
                  "WHERE ca.alias LIKE :str "
                  "GROUP BY c.id "
                  "ORDER BY ca.alias ASC");
    query.bindValue(0, like);
    return query;
}

bool CodeFinder::next_row(Code &code) {
    QSqlQuery lq = this->last_query();

    if(lq.next()) {
        code.id         = lq.value(0).toInt();
        code.name       = lq.value(1).toString();
        code.short_name = lq.value(2).toString();
        code.size       = lq.value(3).toUInt();
        return true;
    }

    return false;
}
