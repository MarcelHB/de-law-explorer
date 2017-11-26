/* SPDX-License-Identifier: GPL-3.0 */

#ifndef CODE_BATCH_LOOKUP_JSON_SERIALIZER_H
#define CODE_BATCH_LOOKUP_JSON_SERIALIZER_H

#include <vector>

#include "json_serializer.h"
#include "../db/orm_structs/code_atom.h"

class CodeBatchLookupJSONSerializer : public JSONSerializer {
  public:
    CodeBatchLookupJSONSerializer();
    void serialize();
    void path(const std::vector<CodeAtom>&);
    void end();
    void start();
  private:
    bool opened;
};

#endif // CODE_BATCH_LOOKUP_JSON_SERIALIZER_H
