/* SPDX-License-Identifier: GPL-3.0 */

#include "no_search_results_controller.h"

NoSearchResultsController::NoSearchResultsController() :
  ws(nullptr)
{}

bool NoSearchResultsController::action(ControllerArgs &args, ControllerResponse &r,
                                 const QString &replay_id, const bool takeover,
                                 const bool already_stacked) {
    NoSearchResultsControllerArgs& ncrargs = dynamic_cast<NoSearchResultsControllerArgs&>(args);

    QString json("{\"query\":\"");
    json.append(ncrargs.query);
    json.append("\"}");
    r.succesful_response(json);

    if(takeover && !already_stacked && this->ws != nullptr) {
        WorkStackItem wsi = {
            QString("Suche"),
            QString("#028DFF"),
            CONTROLLER_NO_SEARCH_RESULTS,
            &ncrargs,
            0,
            QString(replay_id)
        };
        this->ws->push(wsi);
    }

    return true;
}

void NoSearchResultsController::initialize_for_widgets(WorkStack &ws) {
    this->ws = &ws;
}

void NoSearchResultsController::stack_action(const ControllerArgs&, const int sy) {

}

void NoSearchResultsControllerArgs::to_js_object(QMap<QString, QVariant> &args) const {
    args.insert("query", QVariant(this->query));
}
