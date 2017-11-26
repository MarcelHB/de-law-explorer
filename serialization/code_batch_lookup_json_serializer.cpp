/* SPDX-License-Identifier: GPL-3.0 */

#include "code_batch_lookup_json_serializer.h"

CodeBatchLookupJSONSerializer::CodeBatchLookupJSONSerializer() :
  JSONSerializer(),
  opened(false)
{}

void CodeBatchLookupJSONSerializer::serialize() {

}

void CodeBatchLookupJSONSerializer::path(const std::vector<CodeAtom> &path) {
    if(!this->opened) {
        start();
        this->opened = true;
    }

    if(path.size() > 0) {
        const CodeAtom& first = path[0];

        this->key(QString::number(first.id).toStdString());
        this->array_new();

        for(auto it = path.cbegin(); it != path.cend(); it++) {
            const CodeAtom& atom = (*it);
            this->object_new();
            this->key(std::string("id"));
            this->value(atom.id);
            this->key(std::string("key"));
            this->value(atom.key.toStdString());
            this->key(std::string("atom_type"));
            this->value(atom.atomType);
            this->key(std::string("text"));
            this->value(atom.text.toStdString());
            this->object_close();
        }

        this->array_close();
    }
}

void CodeBatchLookupJSONSerializer::start() {
    this->object_new();
}

void CodeBatchLookupJSONSerializer::end() {
    this->object_close();
}
