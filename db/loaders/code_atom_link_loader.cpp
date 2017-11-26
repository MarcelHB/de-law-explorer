/* SPDX-License-Identifier: GPL-3.0 */

#include "code_atom_link_loader.h"

CodeAtomLinkLoader::CodeAtomLinkLoader(vector<int> _ids) :
    ids(_ids)
{}

QString CodeAtomLinkLoader::buildQuery() {
    std::stringstream sstm;
    sstm << "SELECT code_atom_id, to_code_atom_id, name, start, stop FROM code_links WHERE code_atom_id IN (";

    for(uint32_t i = 0; i < ids.size() - 1; i++)
        sstm << ":id,";

    sstm << ":id)";

    return QString::fromStdString(sstm.str());
}

QSqlQuery CodeAtomLinkLoader::query() {
    QSqlDatabase *db = default_db();
    QSqlQuery query(*db);
    query.prepare(buildQuery());
    for(uint32_t i = 0; i < ids.size(); i++)
        query.bindValue(i, ids[i]);
    return query;
}

bool CodeAtomLinkLoader::next_row(QMap<uint32_t, vector<CodeAtomLink>> &linkMap) {
    QSqlQuery lq = this->last_query();

    if(lq.next()) {
        CodeAtomLink codeAtomLink;
        codeAtomLink.codeAtomId = lq.value(0).toInt();
        codeAtomLink.toCodeAtomId = lq.value(1).toInt();
        codeAtomLink.name = lq.value(2).toString();
        codeAtomLink.start = lq.value(3).toInt();
        codeAtomLink.stop = lq.value(4).toInt();

        if(!linkMap.contains(codeAtomLink.codeAtomId))
            linkMap.insert(codeAtomLink.codeAtomId, vector<CodeAtomLink>());

        linkMap[codeAtomLink.codeAtomId].push_back(codeAtomLink);

        return true;
    }

    return false;
}
