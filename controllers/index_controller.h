/* SPDX-License-Identifier: GPL-3.0 */

#ifndef DE_LAW_EXPLORER_INDEX_CONTROLLER_H
#define DE_LAW_EXPLORER_INDEX_CONTROLLER_H

#include "controller.h"
#include "controller_response.h"

#define CONTROLLER_INDEX        1

class IndexControllerArgs;

class IndexController : public Controller {
  public:
    IndexController();

    bool action(ControllerArgs&, ControllerResponse&, const QString&, const bool, const bool);
    void initialize_for_widgets(WorkStack&);
    void stack_action(const ControllerArgs&, const int);
    bool uses_gl() const;
  private:
    WorkStack *ws;

    void to_stack(IndexControllerArgs&, const QString&);
};

class IndexControllerArgs : public ControllerArgs {
  public:
    IndexControllerArgs() : starts_with('a') {}
    IndexControllerArgs(const char s) : starts_with(s) {}

    void from_js_object(const QMap<QString, QVariant>&);
    void to_js_object(QMap<QString, QVariant>&) const;

    char starts_with;
};

#endif
