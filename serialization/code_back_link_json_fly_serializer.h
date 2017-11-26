/* SPDX-License-Identifier: GPL-3.0 */

#ifndef CODE_BACK_LINK_JSON_FLY_SERIALIZER_H
#define CODE_BACK_LINK_JSON_FLY_SERIALIZER_H

#include "json_serializer.h"
#include "../db/orm_structs/code_atom_link.h"

class CodeBackLinkJSONFlySerializer : public JSONSerializer {
public:
    CodeBackLinkJSONFlySerializer();
    void serialize();
    void code(const CodeAtomLink&);
    void end();
    void start();

private:

    bool opened;
};

#endif // CODE_BACK_LINK_JSON_FLY_SERIALIZER_H
