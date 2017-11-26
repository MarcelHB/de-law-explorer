/* SPDX-License-Identifier: GPL-3.0 */

#define USE_GLEW_NOT_QT
#include "scene_code.h"

DefaultChipRenderer* SceneCode::renderer = nullptr;
size_t SceneCode::inst_count = 0;

SceneCode::SceneCode(const Code &code) :
  _db_id(code.id),
  _scale_factor(this->calc_scale_factor(code.size)),
  texture(NULL)
{
    inst_count++;

    for(auto it = code.short_name.begin(); it != code.short_name.end(); ++it) {
        const uint16_t unicode = *reinterpret_cast<const uint16_t*>(it);
        this->name_codes.push_back(unicode);
    }
}

SceneCode::~SceneCode() {
    if(this->texture != NULL) {
        delete this->texture;
    }

    inst_count--;
    if(inst_count == 0) {
        delete renderer;
        renderer = nullptr;
    }
}

GeometryBoundingType SceneCode::bounding_type() {
    return this->get_renderer().geometry().bounding_type();
}

float SceneCode::calc_scale_factor(uint32_t size) const {
    float scale = std::log(size)/5.0f + 0.5f;
    scale = std::min(scale, 2.0f);
    scale = std::max(scale, 0.5f);
    return scale;
}

std::vector<float>& SceneCode::bounding_data() {
    return this->get_renderer().geometry().bounding_data();
}

uint32_t SceneCode::db_id() const {
    return this->_db_id;
}

void SceneCode::draw(const glm::mat4 &c, const glm::mat4 &p, const glm::vec3 &cp) {
    if(this->texture == NULL) {
        this->texture = this->get_renderer().generate_texture(this->name_codes);
        this->texture->to_gpu();
    }
    this->get_renderer().draw(this->matrix, c, p, cp, *this->texture);
}

const glm::mat4& SceneCode::model_matrix() const {
    return this->matrix;
}

void SceneCode::model_matrix(const glm::mat4& mat) {
    this->matrix = mat;
}

float SceneCode::scale_factor() const {
    return this->_scale_factor;
}

uint32_t SceneCode::type() const {
    return ELEMENT_TYPE_CODE;
}

DefaultChipRenderer& SceneCode::get_renderer() {
    if(renderer == nullptr) {
        renderer = new DefaultChipRenderer;
        renderer->gl_ready();
    }

    return *renderer;
}
