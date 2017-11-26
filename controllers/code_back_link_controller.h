/* SPDX-License-Identifier: GPL-3.0 */

#ifndef CODE_BACK_LINK_CONTROLLER_H
#define CODE_BACK_LINK_CONTROLLER_H

#include "controller.h"
#include "controller_response.h"

#include "../3D/scene/simple_scene_writer.h"

#define CONTROLLER_CODE_BACK_LINK    4

class CodeBackLinkControllerArgs;

class CodeBackLinkController : public Controller {
  public:
    bool action(ControllerArgs&, ControllerResponse&, const QString&, const bool, const bool);
    void stack_action(const ControllerArgs&, const int);
    bool uses_gl() const;
};

class CodeBackLinkControllerArgs : public ControllerArgs {
  public:
    CodeBackLinkControllerArgs() : code_atom_id(1) {}
    CodeBackLinkControllerArgs(uint32_t id) : code_atom_id(id) {}

    void from_js_object(const QMap<QString, QVariant>&);
    void to_js_object(QMap<QString, QVariant>&) const;

    uint32_t code_atom_id;
};

#endif // CODE_BACK_LINK_CONTROLLER_H
