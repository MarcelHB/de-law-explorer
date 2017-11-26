/* SPDX-License-Identifier: GPL-3.0 */

#include "router.h"

#include "index_controller.h"
#include "code_index_controller.h"
#include "code_atom_controller.h"
#include "code_back_link_controller.h"
#include "no_search_results_controller.h"
#include "inter_links_controller.h"
#include "code_controller.h"
#include "code_atom_batch_lookup_controller.h"

Router::Router(WorkStack &ws, WebView &_wv) :
  current_controller(nullptr),
  work_stack(ws),
  wv(_wv)
{}

Router::~Router() {
    this->dispose_current_controller();
}

void Router::dispose_current_controller() {
    if(this->current_controller) {
        delete this->current_controller;
        this->current_controller = nullptr;
    }
}

Controller* Router::get_controller_by_id(const size_t id) {
    Controller *ctrl = nullptr;

    switch(id) {
        case CONTROLLER_INDEX:
            ctrl = new IndexController;
            break;
        case CONTROLLER_CODE_INDEX:
            ctrl = new CodeIndexController;
            break;
        case CONTROLLER_CODE_ATOM:
            ctrl = new CodeAtomController;
            break;
        case CONTROLLER_CODE_BACK_LINK:
            ctrl = new CodeBackLinkController;
            break;
        case CONTROLLER_NO_SEARCH_RESULTS:
            ctrl = new NoSearchResultsController;
            break;
        case CONTROLLER_INTER_CODE_LINKS:
            ctrl = new InterLinksController;
            break;
        case CONTROLLER_CODE:
            ctrl = new CodeController;
            break;
        case CONTROLLER_ATOM_BATCH_LOOKUP:
            ctrl = new CodeAtomBatchLookupController;
            break;
        default:
            throw new RouterPathException(static_cast<int32_t>(id));
            break;
    }

    return ctrl;
}

void Router::request_controller(const QString &req_id, const size_t id, ControllerArgs &args,
                                bool takeover, bool on_stack) {
    Controller *ctrl = this->get_controller_by_id(id);

    if(takeover) {
        if(!on_stack) {
            this->work_stack.clear();
        }
        ctrl->initialize_for_widgets(this->work_stack);
        emit this->controller_changed(ctrl, &args);
    }

    ControllerRunner *cr = new ControllerRunner(req_id, *ctrl, args, takeover,
                                                on_stack, this->primary_mutex);

    this->connect_with_runner(*cr);
    QThreadPool::globalInstance()->start(cr);
}

/* Do not do the web action, just lookup and signalling. */
void Router::request_controller_softly(const WorkStackItem &wsi) {
    Controller *ctrl = this->get_controller_by_id(wsi.ctrl_id);
    ctrl->initialize_for_widgets(this->work_stack);

    emit this->controller_changed(ctrl, wsi.args);
    this->dispose_current_controller();
    this->current_controller = ctrl;
}

void Router::stack_controller(const QString &req_id, const size_t id, ControllerArgs &args_n,
                              ControllerArgs &args_p, bool on_stack) {
    Controller *ctrl = this->get_controller_by_id(id);

    if(this->current_controller) {
        this->current_controller->stack_action(args_p, this->wv.scroll_height());
    }

    ctrl->initialize_for_widgets(this->work_stack);
    emit this->controller_changed(ctrl, &args_n);

    ControllerRunner *cr = new ControllerRunner(req_id, *ctrl, args_p, args_n,
                                                on_stack, this->primary_mutex);

    this->connect_with_runner(*cr);
    QThreadPool::globalInstance()->start(cr);
}

void Router::connect_with_runner(const ControllerRunner &cr) const {
    connect(&cr, SIGNAL(action_finished(const ControllerResponse&)),
            this, SIGNAL(controller_finished(const ControllerResponse&)));
    connect(&cr, SIGNAL(action_to_dispose(Controller*, ControllerResponse*, ControllerArgs*, ControllerArgs*, bool, bool)),
            this, SLOT(on_action_to_dispose(Controller*, ControllerResponse*, ControllerArgs*, ControllerArgs*, bool, bool)));
}

void Router::on_action_to_dispose(Controller *c, ControllerResponse *cr, ControllerArgs *args_n,
                                  ControllerArgs *args_p, bool keep_args, bool took_over) {
    delete cr;

    if(!keep_args) {
        delete args_n;
    }

    if(args_p != nullptr) {
        delete args_p;
    }

    if(!took_over) {
        delete c;
    } else {
        this->dispose_current_controller();
        this->current_controller = c;
    }
}

RouterPathException::RouterPathException(int32_t code) :
  ExceptionWithCode(code)
{}

const char* RouterPathException::what() const throw() {
    return "Invalid controller ID";
}
