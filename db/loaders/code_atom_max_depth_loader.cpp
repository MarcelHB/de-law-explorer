/* SPDX-License-Identifier: GPL-3.0 */

#include "code_atom_max_depth_loader.h"

CodeAtomMaxDepthLoader::CodeAtomMaxDepthLoader(QVector<uint32_t> &_ids) :
  ids(_ids)
{}

QSqlQuery CodeAtomMaxDepthLoader::query() {
    QSqlDatabase *db = default_db();
    QSqlQuery query(*db);
    /* Inserting the joined string into this query via bindValue won't work :( */
    QString query_string("SELECT MAX(depth) AS depth FROM "
                         "code_atoms ca "
                         "WHERE ca.id IN(");

    QStringList list;
    this->ids_to_stringlist(list);

    query_string.append(list.join(","));
    query_string.append(")");

    query.prepare(query_string);

    return query;
}

bool CodeAtomMaxDepthLoader::next_row(uint32_t &depth) {
    QSqlQuery lq = this->last_query();

    if(lq.next()) {
        depth = lq.value(0).toUInt();
        return true;
    }

    return false;
}

void CodeAtomMaxDepthLoader::ids_to_stringlist(QStringList &list) {
    for(auto it = this->ids.cbegin(); it != this->ids.cend(); ++it) {
        uint32_t id = *it;
        QString id_string = QString::number(id);
        list.append(id_string);
    }
}
