/* SPDX-License-Identifier: GPL-3.0 */

#ifndef _H_JURA_CODE_ATOM_TREE_JSON_SERIALIZER
#define _H_JURA_CODE_ATOM_TREE_JSON_SERIALIZER

#include <vector>
#include <QMap>

#include "json_serializer.h"
#include "../db/orm_structs/code_atom_node.h"

class CodeAtomTreeJSONSerializer : public JSONSerializer {
  public:
    CodeAtomTreeJSONSerializer(const CodeAtomNode&,
                               const QMap<uint32_t, std::vector<CodeAtomLink>> &links);
    void serialize();
  private:
    const CodeAtomNode *root;
    const QMap<uint32_t, std::vector<CodeAtomLink>> *links;
    void write_code_atom_attrs(const CodeAtom&);
    void write_links(const CodeAtom&);
};

struct CodeAtomTreeJSONSerializerStackElem {
    std::map<uint32_t, CodeAtomNode>::const_iterator current;
    std::map<uint32_t, CodeAtomNode>::const_iterator end;
    bool open;
};

#endif
