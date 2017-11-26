/* SPDX-License-Identifier: GPL-3.0 */

#include "json_serializer.h"

JSONSerializer::JSONSerializer() :
  state(CLEAN)
{}

std::stringstream& JSONSerializer::stringstream() {
    return this->os;
}

void JSONSerializer::key(const std::string& str) {
    this->next_key_comma();
    os.write("\"", 1);
    os.write(str.c_str(), str.size());
    os.write("\":", 2);
    this->state = KEY;
}

void JSONSerializer::array_new() {
    this->next_comma();
    os.write("[", 1);
    this->state = CLEAN;
}

void JSONSerializer::array_close() {
    os.write("]", 1);
    this->state = NEXT;
}

void JSONSerializer::object_new() {
    this->next_comma();
    os.write("{", 1);
    this->state = CLEAN;
}

void JSONSerializer::object_close() {
    os.write("}", 1);
    this->state = NEXT;
}

void JSONSerializer::value(const int v) {
    this->next_comma();
    os << v;
    this->state = NEXT;
}

void JSONSerializer::value(const uint32_t v) {
    this->next_comma();
    os << v;
    this->state = NEXT;
}

void JSONSerializer::value(const bool v) {
    this->next_comma();
    const char *str = v ? "true" : "false";
    const size_t len = v ? 4 : 5;
    os.write(str, len);
    this->state = NEXT;
}

void JSONSerializer::value(const std::string& str) {
    this->next_comma();
    os.write("\"", 1);
    for(auto it = str.cbegin(); it != str.cend(); ++it) {
        const char c = *it;
        if(c == '"') {
            os.write("\\", 1);
        }
        os.write(&c, 1);
    }
    os.write("\"", 1);
    this->state = NEXT;
}

void JSONSerializer::null() {
    if(this->state == NEXT) {
        os.write(",", 1);
    }
    os.write("null", 4);
    this->state = NEXT;
}

void JSONSerializer::next_comma() {
    if(this->state == NEXT) {
        os.write(",", 1);
    }
}

void JSONSerializer::next_key_comma() {
    if(this->state == KEY || this->state == NEXT) {
        os.write(",", 1);
    }
}
