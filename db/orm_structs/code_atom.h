/* SPDX-License-Identifier: GPL-3.0 */

#ifndef CODE_ATOM_H
#define CODE_ATOM_H

#include <list>
#include <QString>

#include "code_atom_link.h"

struct CodeAtom {
    uint32_t id;
    int codeId;
    int position;
    QString key;
    int atomType;
    int depth;
    size_t treeDepth;
    bool joined;
    QString text;
    std::list<CodeAtomLink> links;
    std::list<CodeAtom> children;
};

#endif // CODE_ATOM_H
