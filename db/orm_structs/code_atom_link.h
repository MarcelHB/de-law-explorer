/* SPDX-License-Identifier: GPL-3.0 */

#ifndef CODE_ATOM_LINK_H
#define CODE_ATOM_LINK_H

#include <QString>

struct CodeAtomLink {
    uint32_t id;
    uint32_t codeAtomId;
    uint32_t toCodeAtomId;
    uint32_t codeId;
    QString name;
    QString revName;
    int start;
    int stop;
};

#endif // CODE_ATOM_LINK_H
