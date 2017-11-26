/* SPDX-License-Identifier: GPL-3.0 */

#include <QThread>

#include "controller_runner.h"
#include "controller_response.h"

ControllerRunner::ControllerRunner(const QString &req_id, Controller &ctrl,
                                   ControllerArgs &_args, const bool _takeover,
                                   const bool _stacked, std::mutex& m) :
    controller(ctrl),
    request_id(req_id),
    args_n(_args),
    args_p(nullptr),
    takeover(_takeover),
    stacked(_stacked),
    mutex(m)
{}

ControllerRunner::ControllerRunner(const QString &req_id, Controller &ctrl,
                                   ControllerArgs &_args_p, ControllerArgs &_args_n,
                                   const bool _stacked, std::mutex& m) :
    controller(ctrl),
    request_id(req_id),
    args_n(_args_n),
    args_p(&_args_p),
    takeover(true),
    stacked(_stacked),
    mutex(m)
{}

void ControllerRunner::run() {
    ControllerResponse *r = new ControllerResponse(this->request_id);
    this->mutex.lock();
    bool keep_args = this->controller.action(this->args_n, *r, this->request_id, takeover, this->stacked);

    emit this->action_finished(*r);
    emit this->action_to_dispose(&this->controller, r, &this->args_n, this->args_p, keep_args, this->takeover);
    this->mutex.unlock();
}
