/* SPDX-License-Identifier: GPL-3.0 */

#include "code_atom_finder.h"

CodeAtomFinder::CodeAtomFinder(const QString &str, uint32_t _code_id,
                               CodeAtomType _type) :
  pattern(str),
  code_id(_code_id),
  type(_type)
{}

QSqlQuery CodeAtomFinder::query() {
    QSqlDatabase *db = default_db();
    QSqlQuery query(*db);

    query.prepare("SELECT ca.id, ca.position, ca.key, ca.atom_type, ca.depth, ca.tree_depth, "
                  "ca.code_id, ca.text "
                  "FROM code_atoms ca "
                  "WHERE ca.key LIKE :pattern AND ca.code_id = :id AND ca.atom_type = :type");
    query.bindValue(0, this->pattern);
    query.bindValue(1, this->code_id);
    query.bindValue(2, this->type);

    return query;
}

bool CodeAtomFinder::next_row(CodeAtom &ca) {
    QSqlQuery lq = this->last_query();

    if(lq.next()) {
        ca.id =         lq.value(0).toInt();
        ca.position =   lq.value(1).toUInt();
        ca.key =        lq.value(2).toString();
        ca.atomType =   lq.value(3).toInt();
        ca.depth =      lq.value(4).toInt();
        ca.treeDepth =  lq.value(5).toInt();
        ca.codeId =     lq.value(6).toInt();
        ca.text =       lq.value(7).toString();

        return true;
    }

    return false;
}
