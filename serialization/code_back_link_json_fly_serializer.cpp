/* SPDX-License-Identifier: GPL-3.0 */

#include "code_back_link_json_fly_serializer.h"

CodeBackLinkJSONFlySerializer::CodeBackLinkJSONFlySerializer() :
  JSONSerializer(),
  opened(false)
{}

void CodeBackLinkJSONFlySerializer::serialize() {
    // Nada
}

void CodeBackLinkJSONFlySerializer::code(const CodeAtomLink& link) {
    if(!this->opened) {
        start();
        this->opened = true;
    }

    this->object_new();
    this->key(std::string("code_atom_id"));
    this->value(link.codeAtomId);
    this->key(std::string("rev_name"));
    this->value(link.revName.toStdString());
    this->object_close();
}

void CodeBackLinkJSONFlySerializer::end() {
    this->array_close();
    this->object_close();
}

void CodeBackLinkJSONFlySerializer::start() {
    this->object_new();
    this->key(std::string("records"));
    this->array_new();
}
