/* SPDX-License-Identifier: GPL-3.0 */

#ifndef DE_LAW_EXPLORER_CODE_ATOM_CONTROLLER_H
#define DE_LAW_EXPLORER_CODE_ATOM_CONTROLLER_H

#include <string>

#include "controller.h"
#include "controller_response.h"

#include "../db/orm_structs/code.h"
#include "../db/orm_structs/code_atom_node.h"
#include "../3D/scene/simple_scene_writer.h"

#define CONTROLLER_CODE_ATOM    3

class CodeAtomControllerArgs;

class CodeAtomController : public Controller {
  Q_OBJECT
  public:
    CodeAtomController();
    ~CodeAtomController();

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
    void to_stack(CodeAtomControllerArgs&, const CodeAtom&, const QString&);
    QString json_tree(CodeAtomNode&) const;
    const char* stack_color(const CodeAtom&) const;
    QString stack_name(const CodeAtom&) const;
  private slots:
    void on_scene_inter_links_request(const uint32_t, const uint32_t);
};

class CodeAtomControllerArgs : public ControllerArgs {
  public:
    CodeAtomControllerArgs() : code_atom_id(1) {}
    CodeAtomControllerArgs(uint32_t id) : code_atom_id(id) {}

    void from_js_object(const QMap<QString, QVariant>&);
    void to_js_object(QMap<QString, QVariant>&) const;

    uint32_t code_atom_id;
};

#endif
