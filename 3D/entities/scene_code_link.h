/* SPDX-License-Identifier: GPL-3.0 */

#ifndef SCENE_CODE_LINK_H
#define SCENE_CODE_LINK_H

#include <vector>
#include <cstdint>

#include "../geometry/chip_geometry_provider.h"
#include "../scene/scene_element.h"
#include "../../db/orm_structs/code.h"
#include "../../db/orm_structs/code_neighbour.h"

#define ELEMENT_TYPE_CODE_LINK 0x00000002

class SceneCodeLink : public SceneElement {
  public:
    SceneCodeLink(const Code&, const CodeNeighbour&);
    SceneCodeLink(const Code&, const CodeNeighbour&, const CodeNeighbour&);

    GeometryBoundingType bounding_type();
    std::vector<float>& bounding_data();
    uint32_t code_a_id() const;
    uint32_t code_b_id() const;
    void draw(const glm::mat4&, const glm::mat4&, const glm::vec3&);
    const glm::mat4& model_matrix() const;
    void model_matrix(const glm::mat4&);
    uint32_t type() const;
  private:
    uint32_t _code_a_id;
    uint32_t _code_b_id;
    bool from;
    bool to;
    size_t from_count;
    size_t to_count;
    glm::mat4 matrix;
};

#endif // SCENE_CODE_LINK_H
