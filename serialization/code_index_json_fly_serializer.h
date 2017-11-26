/* SPDX-License-Identifier: GPL-3.0 */

#ifndef CODE_INDEX_JSON_SERIALIZER_H
#define CODE_INDEX_JSON_SERIALIZER_H

#include "json_serializer.h"
#include "../db/orm_structs/code.h"

class CodeIndexJSONFlySerializer : public JSONSerializer {
public:
    CodeIndexJSONFlySerializer();
    void serialize();
    void code(const Code&);
    void end();
    void start();

private:
    bool opened;
};

#endif // CODE_INDEX_JSON_SERIALIZER_H
