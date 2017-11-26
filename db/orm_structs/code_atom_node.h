/* SPDX-License-Identifier: GPL-3.0 */

#ifndef CODE_ATOM_NODE_H
#define CODE_ATOM_NODE_H

#include <cstdint>
#include <map>

#include "code_atom.h"

struct CodeAtomNode {
    CodeAtom code_atom;
    std::map<uint32_t, CodeAtomNode> children;
};

#endif // CODE_ATOM_NODE_H
