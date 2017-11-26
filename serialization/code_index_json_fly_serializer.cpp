/* SPDX-License-Identifier: GPL-3.0 */

#include "code_index_json_fly_serializer.h"

CodeIndexJSONFlySerializer::CodeIndexJSONFlySerializer() :
  JSONSerializer(),
  opened(false)
{}

void CodeIndexJSONFlySerializer::serialize() {
    // Nada
}

void CodeIndexJSONFlySerializer::code(const Code& code) {
    if(!this->opened) {
        start();
        this->opened = true;
    }

    this->object_new();
    this->key(std::string("id"));
    this->value(code.id);
    this->key(std::string("name"));
    this->value(code.name.toStdString());
    this->key(std::string("short"));
    this->value(code.short_name.toStdString());
    this->object_close();
}

void CodeIndexJSONFlySerializer::end() {
    this->array_close();
    this->object_close();
}

void CodeIndexJSONFlySerializer::start() {
    this->object_new();
    this->key(std::string("records"));
    this->array_new();
}
