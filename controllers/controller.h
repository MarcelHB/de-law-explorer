/* SPDX-License-Identifier: GPL-3.0 */

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <cstdint>
#include <QMap>
#include <QVariant>

#include "controller_response.h"
#include "3D/scene/scene_manager.h"
#include "3D/scene/scene_writer.h"

class ControllerArgs {
  public:
    virtual ~ControllerArgs() {}
    virtual void from_js_object(const QMap<QString, QVariant>&) = 0;
    virtual void to_js_object(QMap<QString, QVariant>&) const = 0;
};

#include "components/work_stack.h"

class Controller : public QObject {
  Q_OBJECT
  public:
    virtual ~Controller() {}
    virtual bool action(ControllerArgs&, ControllerResponse&, const QString&,
                        const bool, const bool) = 0;
    virtual void initialize_for_widgets(WorkStack&) {}
    virtual void stack_action(const ControllerArgs&, const int) = 0;
    virtual bool uses_gl() const;
    virtual void gl_action() {}
    virtual SceneWriter* propose_scene_writer(const ControllerArgs&, SceneManager&) {
        return nullptr;
    }
    virtual SceneWriter* continue_scene_writer(const SceneWriter*) {
        return nullptr;
    }
  signals:
    void controller_change_request(const int, const QMap<QString, QVariant>&);
};

#endif
