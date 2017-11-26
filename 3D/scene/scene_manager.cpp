/* SPDX-License-Identifier: GPL-3.0 */

#include "../GL/glew.h"
#include "../glm/gtc/matrix_transform.hpp"
#include "scene_manager.h"

float SceneManager::clear_color[4] = { 0.0f, 0.0f, 0.2f, 1.0f };

SceneManager::SceneManager() :
  SceneManager(0, 0)
{ }

SceneManager::SceneManager(uint32_t w, uint32_t h) :
  ticks(0),
  cam_pos(glm::vec3(0.0f, 4.0f, 5.5f)),
  cam_dir(glm::vec3(0.0f, 0.0f, 0.0f)),
  vp_width(w),
  vp_height(h)
{
    this->calc_matrices();

}

SceneManager::~SceneManager() {
    this->delete_elements();
}

float SceneManager::aspect_ratio() const {
    return (this->vp_width * 1.0f) / this->vp_height;
}

void SceneManager::calc_matrices() {
    this->cam_matrix = glm::lookAt(this->cam_pos,
                                   this->cam_dir,
                                   glm::vec3(0.0f,1.0f,0.0f));
    this->proj_matrix = glm::perspective(FOV,
                                         this->aspect_ratio(),
                                         NEAR_PLANE,
                                         FAR_PLANE);
    this->cp_matrix = this->proj_matrix * this->cam_matrix;

    /* Why is this not working here (did in static demo) ... */
    /* glm::mat4& p = this->proj_matrix;
    float normals[24] = {
        p[3][0] + p[0][0], p[3][1] + p[0][1], p[3][2] + p[0][2], p[3][3] + p[0][3],
        p[3][0] - p[0][0], p[3][1] - p[0][1], p[3][2] - p[0][2], p[3][3] - p[0][3],
        p[3][0] + p[1][0], p[3][1] + p[1][1], p[3][2] + p[1][2], p[3][3] + p[1][3],
        p[3][0] - p[1][0], p[3][1] - p[1][1], p[3][2] - p[1][2], p[3][3] - p[1][3],
        p[3][0] + p[2][0], p[3][1] + p[2][1], p[3][2] + p[2][2], p[3][3] + p[2][3],
        p[3][0] - p[2][0], p[3][1] - p[2][1], p[3][2] - p[2][2], p[3][3] - p[2][3]
    };*/
    /* ... and does this always hold? */
    float normals[24] = {
        1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, -1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, -1.0f, 1.0f
    };
    this->frustum_normals.assign(&normals[0], &normals[0] + 24);
}

void SceneManager::clear_scene() {
    glClearColor(clear_color[0],
                 clear_color[1],
                 clear_color[2],
                 clear_color[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void SceneManager::add_element(SceneElement &elem) {
    AccountedElement ae(elem, false);
    elements.push_back(ae);
}

void SceneManager::delete_elements() {
    for(auto it = elements.begin(); it != elements.end(); ++it) {
        AccountedElement& ae = *it;
        delete ae.element();
    }
    elements.clear();
}

void SceneManager::frame() {
    this->ticks++;

    this->clear_scene();
    this->frustum_check();

    for(auto it = this->elements.begin(); it != this->elements.end(); ++it) {
        AccountedElement& elem = (*it);
        if(elem.visible()) {
            elem.element()->draw(this->cam_matrix, this->proj_matrix, this->cam_pos);
        }
    }
}

void SceneManager::frustum_check() {
    for(auto it = this->elements.begin(); it != this->elements.end(); ++it) {
        AccountedElement& elem = (*it);
        SceneElement *selem = elem.element();
        bool visible = false;

        switch(selem->bounding_type()) {
            case GP_BOX:
                visible = this->frustum_check_box(*selem);
                break;
            default:
                break;
        }

        elem.visible(visible);
    }
}

bool SceneManager::frustum_check_box(SceneElement &elem) {
    std::vector<float>& bbox = elem.bounding_data();
    bool inside = false;
    std::vector<float>& fn = this->frustum_normals;

    for(size_t j = 0; j < 8 && !inside; ++j) {
        glm::mat4 mcp = this->cp_matrix * elem.model_matrix();
        glm::vec4 pos_ms(bbox[24+j*3], bbox[24+j*3+1], bbox[24+j*3+2], 1);
        glm::vec4 pos_ps = mcp * pos_ms;
        glm::vec3 pos_psh = glm::vec3(pos_ps) / pos_ps.w;
        size_t in = 0;

        for(size_t i = 0; i < 6; ++i) {
            glm::vec3 norm(fn[i*4], fn[i*4+1], fn[i*4+2]);
            glm::vec3 norm_loc = fn[i*4+3] * norm;
            glm::vec3 plane_pos = pos_psh - norm_loc;
            if(glm::dot(plane_pos, norm) > 0) {
                break;
            }
            ++in;
        }

        if(in == 6) {
            inside = true;
        }
    }

    return inside;
}

glm::vec3 SceneManager::camera() const {
    return this->cam_pos;
}

void SceneManager::move_camera(const glm::vec3 &v, bool silent) {
    this->cam_pos += v;
    this->calc_matrices();

    if(!silent) {
        emit this->camera_position_changed(this->cam_pos);
    }
}

glm::vec3 SceneManager::look_at() const {
    return this->cam_dir;
}

void SceneManager::look_at(const glm::vec3 &v, bool silent) {
    this->cam_dir = v;
    this->calc_matrices();

    if(!silent) {
        emit this->camera_direction_changed(this->cam_dir);
    }
}

void SceneManager::camera(const glm::vec3 &v, bool silent) {
    this->cam_pos = v;
    this->calc_matrices();

    if(!silent) {
        emit this->camera_position_changed(this->cam_pos);
    }
}

void SceneManager::select_element(const int32_t x, const int32_t y) {
    float z = 1.0f;
    int vp[4];
    int yi = this->vp_height - y;

    glReadPixels(x, yi, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z);
    glm::vec3 in(x, yi, z);
    glGetIntegerv(GL_VIEWPORT, &vp[0]);

    for(auto it = elements.begin(); it != elements.end(); ++it) {
        AccountedElement& elem = (*it);
        bool found = false;

        if(elem.visible()) {
            switch(elem.element()->bounding_type()) {
                case GP_BOX:
                    found = this->select_element_by_box(*elem.element(), in, vp);
                    break;
                default:
                    break;
            }
            if(found) {
                emit this->element_selected(*elem.element());
                break;
            }
        }
    }
}

bool SceneManager::select_element_by_box(SceneElement &elem, glm::vec3 &in, int (&vp)[4]) {
    std::vector<float>& bd = elem.bounding_data();
    glm::mat4 mc = this->cam_matrix * elem.model_matrix();
    glm::vec3 pos_ms = glm::unProject(in, mc, this->proj_matrix, glm::vec4(vp[0], vp[1], vp[2], vp[3]));

    for(size_t i = 0; i < 6; ++i) {
        glm::vec3 norm(bd[i*4], bd[i*4+1], bd[i*4+2]);
        glm::vec3 plane_pos = pos_ms - (bd[i*4+3] * norm);
        /* Sometimes, this is very close to 0 but still > 0? */
        if(glm::dot(plane_pos, norm) > 0.05) {
            return false;
        }
    }

    return true;
}

void SceneManager::update_viewport(const int32_t x, const int32_t y,
                                   const uint32_t w, const uint32_t h) {
    this->vp_width = w;
    this->vp_height = h;
    this->calc_matrices();
    glViewport(x, y, w, h);
}
