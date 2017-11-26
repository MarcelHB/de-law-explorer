/* SPDX-License-Identifier: GPL-3.0 */

#include "code_index_loader.h"

CodeIndexLoader::CodeIndexLoader(const QChar _c) :
    c(_c)
{}

QSqlQuery CodeIndexLoader::query() {
    QSqlDatabase *db = default_db();
    QSqlQuery query(*db);
    query.prepare("SELECT id, name, short FROM codes WHERE UPPER(short_sort) LIKE :char||'%' ORDER BY short_sort ASC");
    query.bindValue(":char", this->c);

    return query;
}

bool CodeIndexLoader::next_row(Code &code) {
    QSqlQuery lq = this->last_query();

    if(lq.next()) {
        code.id         = lq.value(0).toInt();
        code.name       = lq.value(1).toString();
        code.short_name = lq.value(2).toString();
        return true;
    }

    return false;
}
