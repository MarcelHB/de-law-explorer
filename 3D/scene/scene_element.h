/* SPDX-License-Identifier: GPL-3.0 */

#ifndef SCENEELEMENT_H
#define SCENEELEMENT_H

#include <cstdint>

#include "../glm/glm.hpp"
#include "../geometry/geometry_provider.h"

class SceneElement {
  public:
    virtual ~SceneElement() {}
    virtual GeometryBoundingType bounding_type() = 0;
    virtual std::vector<float>& bounding_data() = 0;
    virtual void draw(const glm::mat4&, const glm::mat4&, const glm::vec3&) = 0;
    virtual const glm::mat4& model_matrix() const = 0;
    virtual uint32_t type() const = 0;
};

#endif // SCENEELEMENT_H
