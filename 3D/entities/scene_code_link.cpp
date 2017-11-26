/* SPDX-License-Identifier: GPL-3.0 */

#include "scene_code_link.h"

SceneCodeLink::SceneCodeLink(const Code& from, const CodeNeighbour& other) :
    _code_a_id(from.id),
    _code_b_id(other.code.id),
    from(other.from),
    to(!other.from),
    from_count(0),
    to_count(0)
{
    if(this->from) {
        this->from_count = other.num_links;
    } else {
        this->to_count = other.num_links;
    }
}

SceneCodeLink::SceneCodeLink(const Code& from, const CodeNeighbour& other,
                             const CodeNeighbour& inverse) :
    _code_a_id(from.id),
    _code_b_id(other.code.id),
    from(true),
    to(true),
    from_count(0),
    to_count(0)
{
    if(other.from) {
        this->from_count = other.num_links;
        this->to_count = inverse.num_links;
    } else {
        this->from_count = inverse.num_links;
        this->to_count = other.num_links;
    }
}

GeometryBoundingType SceneCodeLink::bounding_type() {
    return GP_BOX;
}

std::vector<float>& SceneCodeLink::bounding_data() {
    // TODO
    std::vector<float> v;
    return v;
}

uint32_t SceneCodeLink::code_a_id() const {
    return this->_code_a_id;
}

uint32_t SceneCodeLink::code_b_id() const {
    return this->_code_b_id;
}

void SceneCodeLink::draw(const glm::mat4 &c, const glm::mat4 &p, const glm::vec3 &cp) {
    // TODO
}

const glm::mat4& SceneCodeLink::model_matrix() const {
    return this->matrix;
}

void SceneCodeLink::model_matrix(const glm::mat4& mat) {
    this->matrix = mat;
}

uint32_t SceneCodeLink::type() const {
    return ELEMENT_TYPE_CODE_LINK;
}
