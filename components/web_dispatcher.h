/* SPDX-License-Identifier: GPL-3.0 */

#ifndef WEB_DISPATCHER_H
#define WEB_DISPATCHER_H

#include <QObject>

class WebDispatcher;

#include "../controllers/controller.h"
#include "../controllers/controller_runner.h"

/* Acts as data bridge between controllers and JS. Also sanitizes
 * parameters. */

class WebDispatcher : public QObject {
  Q_OBJECT
  signals:
    void dispatcher_ready();
    void dispose_replay(const QString&);
    void initialize_show_request(const int, const QMap<QString, QVariant>&);
    void request_response(const QString&, const int, const QString&);
  public:
    WebDispatcher(Router&);
    ~WebDispatcher();

    void do_request(const WorkStackItem&, const bool) const;
    Q_INVOKABLE void do_request(const QString, const int,
                                const QMap<QString, QVariant>&,
                                const bool) const;
    Q_INVOKABLE void do_stacking_request(const QString, const int, const int,
                                         const QMap<QString, QVariant>&,
                                         const QMap<QString, QVariant>&) const;
    void do_web_request(const int, const QMap<QString, QVariant>&);
    void indicate_ready();
    void request_replay_disposal(const QString&);
  private slots:
    void on_controller_finished(const ControllerResponse&);
  private:
    Router& router;
    QMetaObject::Connection con_controller_finished;

    ControllerArgs* sanitized_arguments(const int, const QMap<QString, QVariant>&) const;
    void connect_with_runner(ControllerRunner&) const;
};

#endif // WEB_DISPATCHER_H
