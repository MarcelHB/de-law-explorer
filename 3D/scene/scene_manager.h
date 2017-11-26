/* SPDX-License-Identifier: GPL-3.0 */

#ifndef DE_LAW_EXPLORER_3D_SCENE_MANAGER_H
#define DE_LAW_EXPLORER_3D_SCENE_MANAGER_H

#include <vector>
#include <QObject>
#include "../glm/glm.hpp"

#include "scene_element.h"
#include "accounted_element.h"

#define NEAR_PLANE 0.1f
#define FAR_PLANE 100.f
#define FOV 70.0f

class SceneManager : public QObject {
  Q_OBJECT

  public:
    SceneManager();
    SceneManager(uint32_t, uint32_t);
    ~SceneManager();

    void add_element(SceneElement&);
    void delete_elements();
    void frame();
    glm::vec3 look_at() const;
    void look_at(const glm::vec3&, bool silent=false);
    glm::vec3 camera() const;
    void camera(const glm::vec3&, bool silent=false);
    void move_camera(const glm::vec3&, bool silent=false);
    void select_element(const int32_t, const int32_t);
    void update_viewport(const int32_t, const int32_t, const uint32_t, const uint32_t);
  signals:
    void element_selected(SceneElement&);
    void camera_direction_changed(const glm::vec3&);
    void camera_position_changed(const glm::vec3&);
  private:
    uint64_t ticks;
    glm::vec3 cam_pos;
    glm::vec3 cam_dir;
    glm::mat4 cam_matrix;
    glm::mat4 proj_matrix;
    glm::mat4 cp_matrix;
    uint32_t vp_width;
    uint32_t vp_height;
    std::vector<float> frustum_normals;
    std::vector<AccountedElement> elements;

    static float clear_color[4];

    float aspect_ratio() const;
    void calc_matrices();
    void clear_scene();
    void frustum_check();
    bool frustum_check_box(SceneElement&);
    bool select_element_by_box(SceneElement&, glm::vec3&, int (&vp)[4]);
};

#endif
