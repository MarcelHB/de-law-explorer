/* SPDX-License-Identifier: GPL-3.0 */

#ifndef _H_JURA_JSON_SERIALIZER
#define _H_JURA_JSON_SERIALIZER

#include <string>
#include <sstream>
#include <cstdint>

enum JSONSerializerState { CLEAN, KEY, NEXT };

class JSONSerializer {
  public:
    JSONSerializer();
    virtual ~JSONSerializer() {}

    virtual void serialize() = 0;
    std::stringstream& stringstream();
  protected:
    std::stringstream os;
    JSONSerializerState state;

    void key(const std::string&);
    void array_new();
    void array_close();
    void object_new();
    void object_close();
    void value(const int);
    void value(const uint32_t);
    void value(const std::string&);
    void value(const bool);
    void null();
    void next_comma();
    void next_key_comma();
};

#endif
