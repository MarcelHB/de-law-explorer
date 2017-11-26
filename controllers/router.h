/* SPDX-License-Identifier: GPL-3.0 */

#ifndef DE_LAW_EXPLORER_ROUTER_H
#define DE_LAW_EXPLORER_ROUTER_H

#include <mutex>
#include <QObject>

class Router;

#include "controller.h"
#include "../exception_with_code.h"
#include "../components/work_stack.h"
#include "../components/web_view.h"

class Router : public QObject {
  Q_OBJECT

  signals:
    void replay_requested(const QString&, const ControllerResponse &r);
    void controller_changed(Controller*, const ControllerArgs*);
    void controller_finished(const ControllerResponse&);
  public:
    Router(WorkStack&, WebView&);
    ~Router();
    void request_controller_softly(const WorkStackItem&);
    void request_controller(const QString&, const size_t, ControllerArgs&,
                            const bool, const bool on_stack=false);
    void stack_controller(const QString&, const size_t, ControllerArgs&, ControllerArgs&,
                          const bool on_stack=false);

  private:
    Controller *current_controller;
    WorkStack& work_stack;
    WebView& wv;
    std::mutex primary_mutex;

    void connect_with_runner(const ControllerRunner&) const;
    void dispose_current_controller();
    Controller* get_controller_by_id(const size_t);

  private slots:
    void on_action_to_dispose(Controller*, ControllerResponse*, ControllerArgs*,
                              ControllerArgs*, bool, bool);
};

class RouterPathException : public ExceptionWithCode {
  public:
    RouterPathException(int32_t);
    const char* what() const throw();
};

#endif
