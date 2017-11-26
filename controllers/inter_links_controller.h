/* SPDX-License-Identifier: GPL-3.0 */

#ifndef INTER_LINKS_CONTROLLER_H
#define INTER_LINKS_CONTROLLER_H

#include "controller.h"
#include "3D/scene/simple_scene_writer.h"

#define CONTROLLER_INTER_CODE_LINKS   6

class InterLinksControllerArgs : public ControllerArgs {
  public:
    InterLinksControllerArgs() {}
    InterLinksControllerArgs(uint32_t, uint32_t);

    void from_js_object(const QMap<QString, QVariant>&);
    void to_js_object(QMap<QString, QVariant>&) const;

    uint32_t from_id;
    uint32_t to_id;
};

class InterLinksController : public Controller {
  Q_OBJECT
  public:
    InterLinksController();
    ~InterLinksController();

    bool action(ControllerArgs&, ControllerResponse&, const QString&, const bool, const bool);
    void initialize_for_widgets(WorkStack&);
    void stack_action(const ControllerArgs&, const int);
    bool uses_gl() const;
    void gl_action();
    SceneWriter* propose_scene_writer(const ControllerArgs&, SceneManager&);
    SceneWriter* continue_scene_writer(const SceneWriter*);
  private:
    WorkStack *ws;
    SimpleSceneWriter *ssw;

    void connect_to_ssw();
    void to_stack(InterLinksControllerArgs&, const QString&);
  private slots:
    void on_scene_inter_links_request(const uint32_t, const uint32_t);
};

#endif // INTER_LINKS_CONTROLLER_H
