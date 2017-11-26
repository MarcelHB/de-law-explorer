/* SPDX-License-Identifier: GPL-3.0 */

#ifndef DE_LAW_EXPLORER_3D_SCENE_CODE_H
#define DE_LAW_EXPLORER_3D_SCENE_CODE_H

#include <vector>
#include <cstdint>

#include "../../db/orm_structs/code.h"
#include "../geometry/chip_geometry_provider.h"
#include "../scene/scene_element.h"
#include "../renderers/default_chip_renderer.h"
#include "../textures/gl_texture.h"

#define ELEMENT_TYPE_CODE 0x00000001

class SceneCode : public SceneElement {
  public:
    SceneCode(const Code&);
    ~SceneCode();

    GeometryBoundingType bounding_type();
    std::vector<float>& bounding_data();
    float calc_scale_factor(uint32_t) const;
    uint32_t db_id() const;
    void draw(const glm::mat4&, const glm::mat4&, const glm::vec3&);
    const glm::mat4& model_matrix() const;
    void model_matrix(const glm::mat4&);
    float scale_factor() const;
    uint32_t type() const;
  private:
    uint32_t _db_id;
    float _scale_factor;
    std::vector<uint16_t> name_codes;
    glm::mat4 matrix;
    GLTexture *texture;

    static size_t inst_count;
    static DefaultChipRenderer *renderer;
    static DefaultChipRenderer& get_renderer();
};

#endif
