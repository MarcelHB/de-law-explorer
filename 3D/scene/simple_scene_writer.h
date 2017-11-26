/* SPDX-License-Identifier: GPL-3.0 */

#ifndef SIMPLE_SCENE_WRITER_H
#define SIMPLE_SCENE_WRITER_H

#include <cstdint>
#include <vector>
#include <QObject>
#include <QMetaObject>

#include "scene_writer.h"
#include "scene_manager.h"
#include "../entities/scene_code.h"
#include "../entities/scene_code_link.h"
#include "../renderers/default_chip_renderer.h"

#define SSW_MAGIC_NUMBER 0x10203040

class SimpleSceneWriter : public QObject, public SceneWriter {
  Q_OBJECT

  public:
    SimpleSceneWriter(uint32_t, SceneManager&);
    SimpleSceneWriter(const SimpleSceneWriter&);
    ~SimpleSceneWriter();
    void write();
  private slots:
    void on_element_selected(SceneElement&);
    void on_camdir_changed(const glm::vec3&);
    void on_campos_changed(const glm::vec3&);
  private:
    uint32_t code_id;
    SceneManager& scene_manager;
    std::vector<SceneCodeLink> links;
    size_t _signature;
    size_t current_circles;
    QMetaObject::Connection con_sm_selected_object;
    QMetaObject::Connection con_sm_camdir_changed;
    QMetaObject::Connection con_sm_campos_changed;

    void connect_to_scene_manager();
    float extreme_x_for_cam() const;
    float extreme_y_for_cam() const;
    float extreme_z_for_cam() const;
    size_t get_circles_num(size_t) const;
    size_t get_elems_per_circle(size_t, size_t, size_t) const;
    size_t signature() const;
  signals:
    void show_inter_links_request(const uint32_t, const uint32_t);
};

#endif // SIMPLE_SCENE_WRITER_H
