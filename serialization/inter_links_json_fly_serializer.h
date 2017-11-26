/* SPDX-License-Identifier: GPL-3.0 */

#ifndef INTER_LINKS_FLY_SERIALIZER_H
#define INTER_LINKS_FLY_SERIALIZER_H

#include "json_serializer.h"
#include "../db/orm_structs/code_atom_link.h"

class InterLinksJSONFlySerializer : public JSONSerializer {
public:
    InterLinksJSONFlySerializer();
    void serialize();
    void code_atom_link(const CodeAtomLink&);
    void end();
    void start();

private:
    bool opened;
};

#endif // INTER_LINKS_FLY_SERIALIZER_H
