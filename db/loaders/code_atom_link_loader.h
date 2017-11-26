#ifndef CODE_ATOM_LINK_LOADER_H
#define CODE_ATOM_LINK_LOADER_H

#include <cstdint>
#include <sstream>

#include "loader.h"
#include "../orm_structs/code_atom_link.h"

using namespace std;

class CodeAtomLinkLoader : public DBLoader<QMap<uint32_t, vector<CodeAtomLink>>> {
public:
    CodeAtomLinkLoader(vector<int>);
    bool next_row(QMap<uint32_t, vector<CodeAtomLink> > &);
protected:
    QSqlQuery query();
private:
    vector<int> ids;

    QString buildQuery();
};

#endif // CODE_ATOM_LINK_LOADER_H
