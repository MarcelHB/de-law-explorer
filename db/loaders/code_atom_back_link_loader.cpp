/* SPDX-License-Identifier: GPL-3.0 */

#include "code_atom_back_link_loader.h"

CodeAtomBackLinkLoader::CodeAtomBackLinkLoader(uint32_t _id) :
    id(_id)
{}

QSqlQuery CodeAtomBackLinkLoader::query() {
    QSqlDatabase *db = default_db();
    QSqlQuery query(*db);
    query.prepare("SELECT code_atom_id, rev_name FROM code_links WHERE to_code_atom_id = :id");
    query.bindValue(0, this->id);
    return query;
}

bool CodeAtomBackLinkLoader::next_row(CodeAtomLink &codeAtomLink) {
    QSqlQuery lq = this->last_query();

    if(lq.next()) {
        codeAtomLink.codeAtomId = lq.value(0).toInt();
        codeAtomLink.revName = lq.value(1).toString();

        return true;
    }

    return false;
}
