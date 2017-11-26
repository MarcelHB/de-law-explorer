/* SPDX-License-Identifier: GPL-3.0 */

#include "code_atom_tree_json_serializer.h"

#include <stack>
#include <iostream>

CodeAtomTreeJSONSerializer::CodeAtomTreeJSONSerializer(const CodeAtomNode &node,
                                                       const QMap<uint32_t, std::vector<CodeAtomLink> > &_links) :
    JSONSerializer(),
    root(&node),
    links(&_links)
{}

void CodeAtomTreeJSONSerializer::serialize() {
    std::stack<CodeAtomTreeJSONSerializerStackElem> it_stack;

    const CodeAtomNode *current_node = this->root;
    this->object_new();
    this->write_code_atom_attrs(this->root->code_atom);

    this->write_links(current_node->code_atom);

    this->key(std::string("children"));
    this->array_new();

    CodeAtomTreeJSONSerializerStackElem re = { current_node->children.cbegin(), current_node->children.cend(), false };
    it_stack.push(re);

    while(it_stack.size() > 0) {
        CodeAtomTreeJSONSerializerStackElem& current_it = it_stack.top();

        if(current_it.current != current_it.end) {
            current_node = &(current_it.current->second);
            this->object_new();
            this->write_code_atom_attrs(current_node->code_atom);

            this->write_links(current_node->code_atom);

            this->key(std::string("children"));
            this->array_new();

            CodeAtomTreeJSONSerializerStackElem se = { current_node->children.cbegin(), current_node->children.cend(), false };
            it_stack.push(se);

            current_it.current++;

        } else {
            it_stack.pop();
            this->array_close();
            this->object_close();
        }
    }
}

void CodeAtomTreeJSONSerializer::write_code_atom_attrs(const CodeAtom& atom) {
    this->key(std::string("id"));
    this->value(atom.id);
    this->key(std::string("atom_type"));
    this->value(atom.atomType);
    this->key(std::string("key"));
    this->value(atom.key.toStdString());
    this->key(std::string("position"));
    this->value(atom.position);
    this->key(std::string("joined"));
    this->value(atom.joined);
    this->key(std::string("text"));
    this->value(atom.text.toStdString());
    this->key(std::string("depth"));
    this->value(atom.depth);
}

void CodeAtomTreeJSONSerializer::write_links(const CodeAtom& atom) {
    this->key(std::string("links"));
    this->array_new();

    std::vector<CodeAtomLink> linkv = links->value(atom.id);
    for(auto it = linkv.cbegin(); it != linkv.cend(); ++it) {
        const CodeAtomLink& l = (*it);
        this->object_new();
        this->key(std::string("to_code_atom_id"));
        this->value(l.toCodeAtomId);
        this->key(std::string("name"));
        this->value(l.name.toStdString());
        this->key(std::string("start"));
        this->value(l.start);
        this->key(std::string("stop"));
        this->value(l.stop);
        this->object_close();
    }

    this->array_close();
}
