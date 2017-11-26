/* SPDX-License-Identifier: GPL-3.0 */

#include "code_atom_root_loader.h"

CodeAtomRootLoader::CodeAtomRootLoader(uint32_t _codeId) :
    codeId(_codeId)
{}

QSqlQuery CodeAtomRootLoader::query() {
    QSqlDatabase *db = default_db();
    QSqlQuery query(*db);
    query.prepare("SELECT ca1.id AS ca1_id, ca1.position AS ca1_position, ca1.key AS ca1_key, ca1.atom_type AS ca1_type, ca1.depth AS ca1_depth, ca1.tree_depth AS ca1_tree_depth, ca1.text  AS ca1_text FROM code_atoms ca1 WHERE ca1.code_id = :codeId AND atom_type = 0");
    query.bindValue(0, this->codeId);

    return query;
}

bool CodeAtomRootLoader::next_row(CodeAtom &codeAtom) {
    QSqlQuery lq = this->last_query();

    if(lq.next()) {
        codeAtom.id = lq.value(0).toInt();
        codeAtom.position = lq.value(1).toUInt();
        codeAtom.key = lq.value(2).toString();
        codeAtom.atomType = lq.value(3).toInt();
        codeAtom.depth = lq.value(4).toInt();
        codeAtom.treeDepth = lq.value(5).toInt();
        codeAtom.text = lq.value(6).toString();

        return true;
    }

    return false;
}
