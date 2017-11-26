/* SPDX-License-Identifier: GPL-3.0 */

#include "code_atom_loader.h"

CodeAtomLoader::CodeAtomLoader(uint32_t _id) :
    id(_id)
{}

QSqlQuery CodeAtomLoader::query() {
    QSqlDatabase *db = default_db();
    QSqlQuery query(*db);
    query.prepare("SELECT ca1.id AS ca1_id, ca1.position AS ca1_position, ca1.key AS ca1_key, "
                  "ca1.atom_type AS ca1_type, ca1.depth AS ca1_depth, ca1.tree_depth AS ca1_tree_depth, "
                  "ca1.code_id, ca1.text  AS ca1_text "
                  "FROM code_atoms ca1 WHERE ca1.id = :id");
    query.bindValue(0, this->id);

    return query;
}

bool CodeAtomLoader::next_row(CodeAtom &codeAtom) {
    QSqlQuery lq = this->last_query();

    if(lq.next()) {
        codeAtom.id = lq.value(0).toInt();
        codeAtom.position = lq.value(1).toUInt();
        codeAtom.key = lq.value(2).toString();
        codeAtom.atomType = lq.value(3).toInt();
        codeAtom.depth = lq.value(4).toInt();
        codeAtom.treeDepth = lq.value(5).toInt();
        codeAtom.codeId = lq.value(6).toInt();
        codeAtom.text = lq.value(7).toString();

        return true;
    }

    return false;
}
