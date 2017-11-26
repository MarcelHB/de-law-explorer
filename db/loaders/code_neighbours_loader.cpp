/* SPDX-License-Identifier: GPL-3.0 */

#include "code_neighbours_loader.h"

CodeNeighboursLoader::CodeNeighboursLoader(uint32_t _id) :
  id(_id)
{}

QSqlQuery CodeNeighboursLoader::query() {
    QSqlDatabase *db = default_db();
    QSqlQuery query(*db);
    query.prepare("SELECT * FROM ( "
                  "SELECT c.id AS id, c.name AS name, c.short AS short, c.size AS size, icl.count AS count, 1 AS 'from' "
                  "FROM inter_code_links icl "
                  "INNER JOIN codes c ON c.id = icl.to_code_id "
                  "WHERE icl.code_id = :id AND c.id <> icl.code_id "
                  "UNION "
                  "SELECT c.id AS id, c.name AS name, c.short AS short, c.size AS size, icl.count AS count, 0 AS 'from' "
                  "FROM inter_code_links icl "
                  "INNER JOIN codes c ON c.id = icl.code_id "
                  "WHERE icl.to_code_id = :id AND c.id <> icl.to_code_id "
                  ") AS c ORDER BY c.short ASC");
    query.bindValue(0, this->id);
    query.bindValue(1, this->id);
    return query;
}

bool CodeNeighboursLoader::next_row(CodeNeighbour &cn) {
    QSqlQuery lq = this->last_query();

    if(lq.next()) {
        cn.code.id         = lq.value(0).toInt();
        cn.code.name       = lq.value(1).toString();
        cn.code.short_name = lq.value(2).toString();
        cn.code.size       = lq.value(3).toUInt();
        cn.num_links       = lq.value(4).toUInt();
        cn.from            = lq.value(5).toBool();
        return true;
    }

    return false;
}
