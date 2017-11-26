/* SPDX-License-Identifier: GPL-3.0 */

#include "inter_links_json_fly_serializer.h"

InterLinksJSONFlySerializer::InterLinksJSONFlySerializer() :
  JSONSerializer(),
  opened(false)
{}

void InterLinksJSONFlySerializer::serialize() {}

void InterLinksJSONFlySerializer::code_atom_link(const CodeAtomLink& cal) {
    if(!this->opened) {
        start();
        this->opened = true;
    }

    this->object_new();
    this->key(std::string("id"));
    this->value(cal.id);
    this->key(std::string("code_atom_id"));
    this->value(cal.codeAtomId);
    this->key(std::string("to_code_atom_id"));
    this->value(cal.toCodeAtomId);
    this->key(std::string("code_id"));
    this->value(cal.codeId);
    this->key(std::string("name"));
    this->value(cal.name.toStdString());
    this->key(std::string("rev_name"));
    this->value(cal.revName.toStdString());
    this->key(std::string("start"));
    this->value(cal.start);
    this->key(std::string("stop"));
    this->value(cal.stop);
    this->object_close();
}

void InterLinksJSONFlySerializer::end() {
    this->array_close();
    this->object_close();
}

void InterLinksJSONFlySerializer::start() {
    this->object_new();
    this->key(std::string("records"));
    this->array_new();
}

