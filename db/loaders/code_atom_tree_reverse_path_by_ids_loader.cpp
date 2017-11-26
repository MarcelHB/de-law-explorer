/* SPDX-License-Identifier: GPL-3.0 */

#include "code_atom_tree_reverse_path_by_ids_loader.h"

CodeAtomTreeReversePathByIDsLoader::CodeAtomTreeReversePathByIDsLoader(QVector<uint32_t> _ids,
                                                                       size_t _depth) :
  ids(_ids),
  depth(_depth)
{}

QString CodeAtomTreeReversePathByIDsLoader::build_query() {
    std::stringstream sstm;

    sstm << "SELECT ca1.id AS ca1_id, ca1.atom_type AS ca1_type, "
         << "ca1.key AS ca1_key, ca1.text AS ca1_text ";

    for(uint32_t i = 0, index = 2; i < this->depth; i++, index++) {
        sstm << ", ca" << index << ".id AS ca" << index << "_id, ca"
             << index << ".atom_type AS ca" << index << "_type, ca"
             << index << ".key AS ca" << index << "_key, ca"
             << index << ".text AS ca" << index << "_text ";
    }

    sstm << "FROM code_atoms ca1 ";

    for(uint32_t i = 0, index = 1; i < this->depth; i++, index++) {
        sstm << "LEFT JOIN code_atom_relations car" << index << index+1
             << " ON car" << index << index+1 << ".code_atom_id = ca"
             << index << ".id AND car" << index << index+1
             << ".virtual = 'f' LEFT JOIN code_atoms ca" << index+1
             << " ON ca" << index+1 << ".id = car" << index << index+1
             << ".parent_code_atom_id ";
    }

    sstm << "WHERE ca1.id IN(";

    for(auto it = this->ids.cbegin(); it != this->ids.cend(); ++it) {
        QString id = QString::number(*it);
        sstm << id.toStdString();

        if(it + 1 != this->ids.cend()) {
            sstm << ",";
        }
    }

    sstm << ")";

    return QString::fromStdString(sstm.str());
}

QSqlQuery CodeAtomTreeReversePathByIDsLoader::query() {
    QSqlDatabase *db = this->default_db();
    QSqlQuery query(*db);

    query.prepare(this->build_query());
    QString q = query.lastQuery();

    return query;
}

bool CodeAtomTreeReversePathByIDsLoader::next_row(std::vector<CodeAtom> &list) {
    QSqlQuery lq = this->last_query();

    if(lq.next()) {
        CodeAtom ca;

        for(size_t i = 0; i <= this->depth; ++i) {
            size_t offset = i * 4;

            if(!lq.value(offset).isNull()) {
                ca.id = lq.value(offset).toUInt();
                ca.atomType = lq.value(offset + 1).toInt();
                ca.key = lq.value(offset + 2).toString();
                ca.text = lq.value(offset + 3).toString();

                list.push_back(ca);
            } else {
                break;
            }
        }

        return true;
    }

    return false;
}
