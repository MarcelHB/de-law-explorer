/* SPDX-License-Identifier: GPL-3.0 */

#include <cstdint>

#include "web_dispatcher.h"
#include "../controllers/index_controller.h"
#include "../controllers/code_atom_controller.h"
#include "../controllers/code_index_controller.h"
#include "../controllers/code_back_link_controller.h"
#include "../controllers/inter_links_controller.h"
#include "../controllers/code_controller.h"
#include "../controllers/code_atom_batch_lookup_controller.h"
#include "../controllers/controller_runner.h"

WebDispatcher::WebDispatcher(Router &r) :
  router(r)
{
    qRegisterMetaType<ControllerResponse>("ControllerResponse");
    this->con_controller_finished = connect(&r, SIGNAL(controller_finished(const ControllerResponse&)),
                                            this, SLOT(on_controller_finished(const ControllerResponse&)));
}

WebDispatcher::~WebDispatcher() {
    disconnect(this->con_controller_finished);
}

void WebDispatcher::do_request(const WorkStackItem &wsi, const bool stacked) const {
    this->router.request_controller(wsi.replay_id, wsi.ctrl_id, *wsi.args, true, stacked);

    return;
}

void WebDispatcher::do_request(const QString request_id, const int ctrl_id,
                               const QMap<QString, QVariant>& args,
                               const bool takeover) const {

    const size_t uctrl_id = static_cast<size_t>(ctrl_id);
    ControllerArgs *ctrl_args = this->sanitized_arguments(uctrl_id, args);

    this->router.request_controller(request_id, ctrl_id, *ctrl_args, takeover, false);

    return;
}

void WebDispatcher::do_stacking_request(const QString request_id, const int next_ctrl_id,
                               const int prev_ctrl_id,
                               const QMap<QString, QVariant>& args_n,
                               const QMap<QString, QVariant>& args_p) const {

    const size_t unctrl_id = static_cast<size_t>(next_ctrl_id);
    const size_t upctrl_id = static_cast<size_t>(prev_ctrl_id);
    ControllerArgs *next_ctrl_args = this->sanitized_arguments(unctrl_id, args_n);
    ControllerArgs *prev_ctrl_args = this->sanitized_arguments(upctrl_id, args_p);

    this->router.stack_controller(request_id, next_ctrl_id, *next_ctrl_args,
                                  *prev_ctrl_args, false);

    return;
}

void WebDispatcher::do_web_request(const int ctrlID, const QMap<QString, QVariant> &args) {
    emit this->initialize_show_request(ctrlID, args);
}

void WebDispatcher::indicate_ready() {
    emit this->dispatcher_ready();
}

void WebDispatcher::on_controller_finished(const ControllerResponse &response) {
     emit this->request_response(response.request_id(), response.return_code(), response.response());
}

void WebDispatcher::request_replay_disposal(const QString &req_id) {
    emit this->dispose_replay(req_id);
}

ControllerArgs* WebDispatcher::sanitized_arguments(const int ctrl_id,
                                                   const QMap<QString, QVariant>& args) const {
    ControllerArgs *ctrl_args = nullptr;

    switch(ctrl_id) {
        case CONTROLLER_INDEX:
            ctrl_args = new IndexControllerArgs;
            break;
        case CONTROLLER_CODE_INDEX:
            ctrl_args = new CodeIndexControllerArgs;
            break;
        case CONTROLLER_CODE_ATOM:
            ctrl_args = new CodeAtomControllerArgs;
            break;
        case CONTROLLER_CODE_BACK_LINK:
            ctrl_args = new CodeBackLinkControllerArgs;
            break;
        case CONTROLLER_INTER_CODE_LINKS:
            ctrl_args = new InterLinksControllerArgs;
            break;
        case CONTROLLER_CODE:
            ctrl_args = new CodeControllerArgs;
            break;
        case CONTROLLER_ATOM_BATCH_LOOKUP:
            ctrl_args = new CodeAtomBatchLookupControllerArgs;
            break;
    }

    if(ctrl_args != nullptr) {
        ctrl_args->from_js_object(args);
    }

    return ctrl_args;
}
