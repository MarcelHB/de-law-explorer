/* SPDX-License-Identifier: GPL-3.0 */

#include "code_loader.h"

CodeLoader::CodeLoader(uint32_t _id) :
  id(_id)
{}

QSqlQuery CodeLoader::query() {
    QSqlDatabase *db = default_db();
    QSqlQuery query(*db);
    query.prepare("SELECT id, name, short, size "
                  "FROM codes WHERE id = :id");
    query.bindValue(0, this->id);
    return query;
}

bool CodeLoader::next_row(Code &code) {
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
