/* SPDX-License-Identifier: GPL-3.0 */

#include "inter_links_loader.h"

InterLinksLoader::InterLinksLoader(uint32_t from, uint32_t to) :
  from_id(from),
  to_id(to)
{}

QSqlQuery InterLinksLoader::query() {
    QSqlDatabase *db = default_db();
    QSqlQuery query(*db);
    query.prepare("SELECT :from_id AS from_code_id, cl.* FROM code_atoms cai "
                  "INNER JOIN code_links cl ON cl.code_atom_id = cai.id "
                  "INNER JOIN code_atoms cao ON cao.id = cl.to_code_atom_id "
                  "WHERE cai.code_id = :from_id AND cao.code_id = :to_id "
                  "UNION "
                  "SELECT :to_id AS from_code_id, cl.* FROM code_atoms cai "
                  "INNER JOIN code_links cl ON cl.code_atom_id = cai.id "
                  "INNER JOIN code_atoms cao ON cao.id = cl.to_code_atom_id "
                  "WHERE cai.code_id = :to_id AND cao.code_id = :from_id ");

    query.bindValue(":from_id", this->from_id);
    query.bindValue(":to_id", this->to_id);

    return query;
}

bool InterLinksLoader::next_row(CodeAtomLink &cal) {
    QSqlQuery lq = this->last_query();

    if(lq.next()) {
        cal.id = lq.value(1).toUInt();
        cal.codeId = lq.value(0).toUInt();
        cal.codeAtomId = lq.value(2).toUInt();
        cal.toCodeAtomId = lq.value(3).toUInt();
        cal.name = lq.value(4).toString();
        cal.revName = lq.value(5).toString();
        cal.start = lq.value(6).toInt();
        cal.stop = lq.value(7).toInt();

        return true;
    }

    return false;
}
