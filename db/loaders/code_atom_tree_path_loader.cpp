/* SPDX-License-Identifier: GPL-3.0 */

#include "code_atom_tree_path_loader.h"

CodeAtomTreePathLoader::CodeAtomTreePathLoader(uint32_t _id, size_t _depth) :
  id(_id),
  depth(_depth)
{}

/* On initial writing, SQLite CTE was not available to Qt/at all so we
 * compile the query based on DB preparation time. */

QString CodeAtomTreePathLoader::build_query() {
    std::stringstream sstm;

    sstm << "SELECT ca1.id AS ca1_id, ca1.position AS ca1_position, ca1.key AS ca1_key, ca1.atom_type AS ca1_type, ca1.depth AS ca1_depth, ca1.tree_depth AS ca1_tree_depth, ca1.text  AS ca1_text";

    for(uint32_t i = 0, index = 2; i < this->depth; i++, index++)
        sstm << ", ca" << index << ".id AS ca" << index << "_id, ca" << index << ".position AS ca" << index << "_position, ca" << index << ".key AS ca" << index << "_key, ca" << index << ".atom_type AS ca" << index << "_type, ca" << index << ".depth AS ca" << index << "_depth, ca" << index << ".tree_depth AS ca" << index << "_tree_depth, ca" << index << ".text  AS ca" << index << "_text";

    sstm << " FROM code_atoms ca1 ";

    for(uint32_t i = 0, index = 1; i < this->depth; i++, index++)
        sstm << "LEFT JOIN code_atom_relations car" << index << index+1 << " ON car" << index << index+1 << ".parent_code_atom_id = ca" << index << ".id AND car" << index << index+1 << ".virtual = 'f' LEFT JOIN code_atoms ca" << index+1 << " ON ca" << index+1 << ".id = car" << index << index+1 << ".code_atom_id ";

    sstm << "WHERE ca1.id = :id";

    return QString::fromStdString(sstm.str());
}

bool CodeAtomTreePathLoader::row_cols_to_code_atom(QSqlQuery &lq, CodeAtom &ca, uint32_t index) {
    if(!lq.value(index*7).isNull()) {
        ca.id = lq.value(index*7).toUInt();
        ca.position = lq.value(1+index*7).toUInt();
        ca.key = lq.value(2+index*7).toString();
        ca.atomType = lq.value(3+index*7).toInt();
        ca.depth = lq.value(4+index*7).toUInt();
        ca.treeDepth = lq.value(5+index*7).toUInt();
        ca.text = lq.value(6+index*7).toString();
        return true;
    }
    return false;
}

QSqlQuery CodeAtomTreePathLoader::query() {
    QSqlDatabase *db = this->default_db();
    QSqlQuery query(*db);
    query.prepare(this->build_query());
    query.bindValue(0, this->id);

    return query;
}

bool CodeAtomTreePathLoader::next_row(std::vector<CodeAtom> &list) {
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
