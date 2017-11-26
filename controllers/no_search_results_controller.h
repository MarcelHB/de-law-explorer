/* SPDX-License-Identifier: GPL-3.0 */

#ifndef NO_SEARCH_RESULTS_CONTROLLER_H
#define NO_SEARCH_RESULTS_CONTROLLER_H

#include "controller.h"
#include "controller_response.h"

#define CONTROLLER_NO_SEARCH_RESULTS 5

class NoSearchResultsController : public Controller {
  public:
    NoSearchResultsController();

    bool action(ControllerArgs&, ControllerResponse&, const QString&, const bool, const bool);
    void initialize_for_widgets(WorkStack&);
    void stack_action(const ControllerArgs&, const int);
  private:
    WorkStack *ws;
};

class NoSearchResultsControllerArgs : public ControllerArgs {
  public:
    NoSearchResultsControllerArgs(const QString &q) : query(q) {}

    /* Should not need this for this ctrl. */
    void from_js_object(const QMap<QString, QVariant>&) {}
    void to_js_object(QMap<QString, QVariant>&) const;

    QString query;
};

#endif // NO_SEARCH_RESULTS_CONTROLLER_H
