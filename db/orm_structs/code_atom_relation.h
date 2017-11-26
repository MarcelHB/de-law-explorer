/* SPDX-License-Identifier: GPL-3.0 */

#ifndef CODE_ATOM_RELATION_H
#define CODE_ATOM_RELATION_H

struct CodeAtomRelation {
    int parentCodeAtomId;
    int codeAtomId;
    bool isVirtual;
};

#endif // CODE_ATOM_RELATION_H
