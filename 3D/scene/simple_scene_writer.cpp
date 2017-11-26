/* SPDX-License-Identifier: GPL-3.0 */

#include <vector>
#include <algorithm>
#include <QObject>

#define USE_GLEW_NOT_QT
#include "simple_scene_writer.h"
#include "../entities/scene_code.h"
#include "../../db/loaders/code_loader.h"
#include "../../db/loaders/code_neighbours_loader.h"
#include "../../glm/gtx/transform.hpp"

SimpleSceneWriter::SimpleSceneWriter(uint32_t _code_id, SceneManager &sm) :
    QObject(),
    code_id(_code_id),
    scene_manager(sm),
    current_circles(0)
{
    this->connect_to_scene_manager();

    std::vector<char32_t> dwords;
    dwords.push_back(static_cast<char32_t>(SSW_MAGIC_NUMBER));
    dwords.push_back(static_cast<char32_t>(_code_id));

    this->_signature = this->create_signature(dwords);
}

SimpleSceneWriter::SimpleSceneWriter(const SimpleSceneWriter &other) :
   QObject(),
   code_id(other.code_id),
   scene_manager(other.scene_manager),
   links(other.links),
   _signature(other._signature),
   current_circles(other.current_circles)
{
    this->connect_to_scene_manager();
}

SimpleSceneWriter::~SimpleSceneWriter() {
    disconnect(this->con_sm_selected_object);
    disconnect(this->con_sm_camdir_changed);
    disconnect(this->con_sm_campos_changed);
}

void SimpleSceneWriter::connect_to_scene_manager() {
    this->con_sm_selected_object = connect(&this->scene_manager, SIGNAL(element_selected(SceneElement&)),
                                           this, SLOT(on_element_selected(SceneElement&)));
    this->con_sm_campos_changed = connect(&this->scene_manager, SIGNAL(camera_position_changed(const glm::vec3&)),
                                           this, SLOT(on_campos_changed(const glm::vec3&)));
    this->con_sm_camdir_changed = connect(&this->scene_manager, SIGNAL(camera_direction_changed(const glm::vec3&)),
                                           this, SLOT(on_camdir_changed(const glm::vec3&)));
}

void SimpleSceneWriter::write() {
    this->scene_manager.delete_elements();
    std::vector<SceneCode*> codes;

    Code center;
    CodeLoader cl(this->code_id);
    cl.load();
    cl.next_row(center);

    SceneCode *scene_center = new SceneCode(center);
    glm::mat4 identity;
    glm::mat4 center_s = glm::scale(identity,
                                    glm::vec3(scene_center->scale_factor(), 1, scene_center->scale_factor()));
    scene_center->model_matrix(identity * center_s);
    codes.push_back(scene_center);

    CodeNeighbour cn;
    CodeNeighbour prev_cn;
    prev_cn.code.id = 0;
    bool one_pending = false;
    CodeNeighboursLoader nl(this->code_id);
    nl.load();

    while(nl.next_row(cn)) {
        /* Group doubles (one from, one to center) */
        if(prev_cn.code.id != 0) {
            SceneCode *code = new SceneCode(prev_cn.code);
            if(one_pending) {
                codes.push_back(code);
            }

            if(prev_cn.code.id != cn.code.id) {
                SceneCodeLink link(center, prev_cn);
                this->links.push_back(link);
                one_pending = true;
            } else {
                SceneCodeLink link(center, prev_cn, cn);
                this->links.push_back(link);
                one_pending = false;
            }
        } else {
            one_pending = true;
        }
        prev_cn = cn;
    }

    if(prev_cn.code.id != 0 && one_pending) {
        SceneCode *code = new SceneCode(prev_cn.code);
        codes.push_back(code);
        SceneCodeLink link(center, prev_cn);
        this->links.push_back(link);
    }

    this->scene_manager.add_element(*codes.front());

    this->current_circles = get_circles_num(codes.size() - 1);
    size_t& circles = this->current_circles;
    size_t total_in = 1;

    for(size_t ci = 0; ci < circles; ++ci) {
        size_t cnum = get_elems_per_circle(ci, circles, codes.size() - 1);
        float rot = M_PI/(std::max(static_cast<size_t>(1), cnum-1));

        for(size_t i = 0; i < cnum; ++i) {
            SceneCode *neighbour = codes.at(total_in + i);
            float sf = neighbour->scale_factor();
            glm::mat4 mat_r = glm::rotate(identity, -rot*i, glm::vec3(0, 1, 0));
            glm::mat4 mat_s = glm::scale(identity, glm::vec3(sf, 1, sf));
            glm::vec4 t(std::max(3.0f, (ci+1) * 5.0f) * -1, 0, 0, 0);
            glm::vec4 tr = mat_r * t;
            glm::mat4 mat_t = glm::translate(identity, glm::vec3(tr));
            neighbour->model_matrix(mat_t * mat_s);
            this->scene_manager.add_element(*neighbour);
        }

        total_in += cnum;
    }

    glm::vec3 center_v(0.0f, 0.0f, -static_cast<float>(circles) * 1.3f);
    this->scene_manager.look_at(center_v);
    glm::vec3 cam_pos(0.0f, circles * 3, 2.0f);
    this->scene_manager.camera(cam_pos);
}

void SimpleSceneWriter::on_camdir_changed(const glm::vec3&) {
    // Nothing for now?
}

void SimpleSceneWriter::on_campos_changed(const glm::vec3 &pos) {
    glm::vec3 new_pos = pos;

    /* X: not too left/right */
    float max_x = this->extreme_x_for_cam();
    if(abs(pos.x) > max_x) {
        new_pos.x = (pos.x < 0 ? -1 : 1) * max_x;
    }

    /* Y: no too high/low */
    if(pos.y < 3) {
        new_pos.y = 3;
    } else {
        float max_y = this->extreme_y_for_cam();
        if(pos.y > max_y) {
            new_pos.y = max_y;
        }
    }

    /* Z: same as x in other dim, but with offset */
    if(pos.z > 0) {
        new_pos.z = 0;
    } else {
        float max_z = this->extreme_z_for_cam();
        if(pos.z < max_z) {
            new_pos.z = max_z;
        }
    }

    this->scene_manager.camera(new_pos, true);
}

void SimpleSceneWriter::on_element_selected(SceneElement &elem) {
    if(elem.type() == ELEMENT_TYPE_CODE) {
        SceneCode& code = dynamic_cast<SceneCode&>(elem);

        if(code.db_id() != this->code_id) {
            emit this->show_inter_links_request(this->code_id, code.db_id());
        }
    }
}

float SimpleSceneWriter::extreme_x_for_cam() const {
    return std::max(3.0f, this->current_circles * 5.0f);
}

float SimpleSceneWriter::extreme_y_for_cam() const {
    /* Prevent elements from falling behind FAR_PLANE. */
    /* TODO: calc. 10.0f */
    return std::min(this->extreme_x_for_cam() * 2.0f, FAR_PLANE - 10.0f);
}

float SimpleSceneWriter::extreme_z_for_cam() const {
    return -this->extreme_x_for_cam();
}

size_t SimpleSceneWriter::get_circles_num(size_t elems) const {
    size_t remainders = elems;
    size_t i = 1;

    while(true) {
        if(remainders / (i*5) > 0) {
            remainders -= i*5;
        } else {
            break;
        }
        i++;
    }

    return i;
}

size_t SimpleSceneWriter::get_elems_per_circle(size_t circle_i, size_t total,
                                               size_t elems) const {
    if(circle_i < total-1) {
        return (circle_i+1)*5;
    } else {
        size_t filled = 0;
        for(size_t i = 0; i < total-1; ++i) {
            filled += (i+1)*5;
        }
        return elems - filled;
    }
}

size_t SimpleSceneWriter::signature() const {
    return this->_signature;
}

