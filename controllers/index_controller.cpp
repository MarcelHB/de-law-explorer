/* SPDX-License-Identifier: GPL-3.0 */

#include <QString>

#include "index_controller.h"
#include "../db/loaders/code_index_loader.h"
#include "../serialization/code_index_json_fly_serializer.h"

IndexController::IndexController() :
  ws(nullptr)
{}

bool IndexController::action(ControllerArgs &args, ControllerResponse &r,
                             const QString &replay_id, const bool takeover,
                             const bool already_stacked) {
    IndexControllerArgs& icargs = dynamic_cast<IndexControllerArgs&>(args);

    CodeIndexLoader cil(icargs.starts_with);
    cil.load();

    CodeIndexJSONFlySerializer fs;
    Code c;
    uint32_t cnt = 0;
    while(cil.next_row(c)) {
        fs.code(c);
        cnt++;
    }

    if(cnt == 0) {
        fs.start();
    }
    fs.end();

    std::stringstream &ss = fs.stringstream();
    std::string str = ss.str();
    QString json = QString::fromStdString(str);

    r.succesful_response(json);
    if(takeover && !already_stacked) {
        this->to_stack(icargs, replay_id);
    }

    return true;
}

void IndexController::initialize_for_widgets(WorkStack &ws) {
    this->ws = &ws;
}

void IndexController::to_stack(IndexControllerArgs &icargs, const QString &replay_id) {
    if(this->ws != nullptr) {
        WorkStackItem wsi = {
            QString("Bücher"),
            QString("#A40000"),
            CONTROLLER_INDEX,
            &icargs,
            0,
            QString(replay_id)
        };
        this->ws->push(wsi);
    }
}

void IndexController::stack_action(const ControllerArgs&, const int sy) {
    WorkStackItem& wsi = this->ws->top();
    wsi.webview_scroll_y = sy;
}

bool IndexController::uses_gl() const {
    return false;
}

void IndexControllerArgs::from_js_object(const QMap<QString, QVariant> &obj) {
    if(obj.contains("startsWith")) {
        QString start_string = obj["startsWith"].toString();
        if(start_string.length() > 0) {
            this->starts_with = start_string.at(0).toLatin1();
        }
    }
}

void IndexControllerArgs::to_js_object(QMap<QString, QVariant> &args) const {
    args.insert("startsWith", QVariant(this->starts_with));
}
