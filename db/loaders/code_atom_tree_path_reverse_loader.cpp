/* SPDX-License-Identifier: GPL-3.0 */

#include "code_atom_tree_path_reverse_loader.h"

CodeAtomTreePathReverseLoader::CodeAtomTreePathReverseLoader(uint32_t _id, size_t _depth) :
    id(_id),
    depth(_depth)
{}

QString CodeAtomTreePathReverseLoader::build_query() {
    std::stringstream sstm;
    sstm << "SELECT ca1.id AS ca1_id, ca1.atom_type AS ca1_type";

    for(uint32_t i = 0, index = 2; i < this->depth; i++, index++)
        sstm << ", ca" << index << ".id AS ca" << index << "_id, ca" << index << ".atom_type AS ca" << index << "_type";

    sstm << " FROM code_atoms ca1 ";

    for(uint32_t i = 0, index = 1; i < this->depth; i++, index++)
        sstm << "LEFT JOIN code_atom_relations car" << index << index+1 << " ON car" << index << index+1 << ".code_atom_id = ca" << index << ".id AND car" << index << index+1 << ".virtual = 'f' LEFT JOIN code_atoms ca" << index+1 << " ON ca" << index+1 << ".id = car" << index << index+1 << ".parent_code_atom_id ";

    sstm << "WHERE ca1.id = :id";

    return QString::fromStdString(sstm.str());
}

bool CodeAtomTreePathReverseLoader::row_cols_to_code_atom(QSqlQuery &lq, CodeAtom &ca, uint32_t index) {
    if(!lq.value(index*2).isNull()) {
        ca.id = lq.value(index*2).toUInt();
        ca.atomType = lq.value(1+index*2).toInt();
        return true;
    }
    return false;
}

QSqlQuery CodeAtomTreePathReverseLoader::query() {
    QSqlDatabase *db = this->default_db();
    QSqlQuery query(*db);
    query.prepare(this->build_query());
    query.bindValue(0, this->id);

    return query;
}

bool CodeAtomTreePathReverseLoader::next_row(std::vector<CodeAtom> &list) {
    QSqlQuery lq = this->last_query();

    if(lq.next()) {
        CodeAtom ca;

        //skip root atom
        for(size_t i = 1; i <= this->depth; ++i) {
            if(this->row_cols_to_code_atom(lq, ca, i)) {
                list.push_back(ca);
            } else {
                break;
            }
        }

        return true;
    }

    return false;
}
