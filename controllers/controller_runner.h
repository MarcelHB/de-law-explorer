/* SPDX-License-Identifier: GPL-3.0 */

#ifndef CONTROLLER_RUNNER_H
#define CONTROLLER_RUNNER_H

#include <mutex>
#include <cstdint>
#include <QObject>
#include <QRunnable>

class ControllerRunner;

#include "router.h"

class ControllerRunner : public QObject, public QRunnable {
  Q_OBJECT
  signals:
    void action_finished(const ControllerResponse&);
    void action_to_dispose(Controller*, ControllerResponse*, ControllerArgs*,
                           ControllerArgs*, bool, bool);
  public:
    ControllerRunner(const QString&, Controller&, ControllerArgs&, bool, bool,
                     std::mutex&);
    ControllerRunner(const QString&, Controller&, ControllerArgs&,
                     ControllerArgs&, bool, std::mutex&);

    void run();
  private:
    Controller& controller;
    const QString request_id;
    ControllerArgs& args_n;
    ControllerArgs* args_p;
    bool takeover;
    bool to_stack;
    bool stacked;
    std::mutex& mutex;
};

#endif // CONTROLLER_RUNNER_H
