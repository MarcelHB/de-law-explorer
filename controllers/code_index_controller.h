/* SPDX-License-Identifier: GPL-3.0 */

#ifndef DE_LAW_EXPLORER_CODE_INDEX_CONTROLLER_H
#define DE_LAW_EXPLORER_CODE_INDEXCONTROLLER_H

#include <string>

#include "controller.h"
#include "controller_response.h"

#include "../db/orm_structs/code.h"
#include "../db/orm_structs/code_atom_node.h"
#include "../3D/scene/simple_scene_writer.h"

#define CONTROLLER_CODE_INDEX   2

class CodeIndexControllerArgs;

class CodeIndexController : public Controller {
  Q_OBJECT
  public:
    CodeIndexController();
    ~CodeIndexController();

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
    void to_stack(CodeIndexControllerArgs&, const Code&, const QString&);
    std::string json_tree(CodeAtomNode&) const;
  private slots:
    void on_scene_inter_links_request(const uint32_t, const uint32_t);
};

class CodeIndexControllerArgs : public ControllerArgs {
  public:
    CodeIndexControllerArgs() : code_id(1) {}
    CodeIndexControllerArgs(uint32_t id) : code_id(id) {}

    void from_js_object(const QMap<QString, QVariant>&);
    void to_js_object(QMap<QString, QVariant>&) const;

    uint32_t code_id;
};

#endif
