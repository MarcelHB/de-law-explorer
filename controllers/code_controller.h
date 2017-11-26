/* SPDX-License-Identifier: GPL-3.0 */

#ifndef CODE_CONTROLLER_H
#define CODE_CONTROLLER_H

#include "controller.h"

#define CONTROLLER_CODE    7

class CodeControllerArgs;

class CodeController : public Controller {
  public:
    bool action(ControllerArgs&, ControllerResponse&, const QString&, const bool, const bool);
    void stack_action(const ControllerArgs&, const int);
    bool uses_gl() const;
};

class CodeControllerArgs : public ControllerArgs {
  public:
    CodeControllerArgs() : code_id(1) {}
    CodeControllerArgs(uint32_t id) : code_id(id) {}

    void from_js_object(const QMap<QString, QVariant>&);
    void to_js_object(QMap<QString, QVariant>&) const;

    uint32_t code_id;
};

#endif // CODE_CONTROLLER_H
