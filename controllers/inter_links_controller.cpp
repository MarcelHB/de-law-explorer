/* SPDX-License-Identifier: GPL-3.0 */

#include "inter_links_controller.h"
#include "../db/loaders/code_loader.h"
#include "../db/loaders/inter_links_loader.h"
#include "../serialization/inter_links_json_fly_serializer.h"

InterLinksController::InterLinksController() :
  ws(nullptr),
  ssw(nullptr)
{}

InterLinksController::~InterLinksController() {
    if(this->ssw != nullptr) {
        delete this->ssw;
    }
}

bool InterLinksController::action(ControllerArgs &args, ControllerResponse &response,
                                  const QString &replay_id, const bool takeover,
                                  const bool already_stacked)
{
    InterLinksControllerArgs& ilcargs = dynamic_cast<InterLinksControllerArgs&>(args);

    InterLinksJSONFlySerializer fs;
    InterLinksLoader ill(ilcargs.from_id, ilcargs.to_id);
    ill.load();

    CodeAtomLink cal;
    uint32_t cnt = 0;
    while(ill.next_row(cal)) {
        fs.code_atom_link(cal);
        cnt++;
    }

    if(cnt == 0) {
        fs.start();
    }
    fs.end();

    std::stringstream &ss = fs.stringstream();
    std::string str = ss.str();
    QString json = QString::fromStdString(str);

    response.succesful_response(json);
    if(takeover && !already_stacked) {
        this->to_stack(ilcargs, replay_id);
    }

    return true;
}

void InterLinksController::to_stack(InterLinksControllerArgs &iclargs,
                                    const QString &replay_id) {
    if(this->ws != nullptr) {
        QString label;

        Code c;
        CodeLoader from_loader(iclargs.from_id);
        from_loader.load();
        if(from_loader.next_row(c)) {
            label.append(c.short_name);
        }
        label.append(":");

        CodeLoader to_loader(iclargs.to_id);
        to_loader.load();
        if(to_loader.next_row(c)) {
            label.append(c.short_name);
        }

        WorkStackItem wsi = {
            label,
            QString("#028DFF"),
            CONTROLLER_INTER_CODE_LINKS,
            &iclargs,
            0,
            QString(replay_id)
        };
        this->ws->push(wsi);
    }
}

void InterLinksController::stack_action(const ControllerArgs&, const int sy) {
    WorkStackItem& wsi = this->ws->top();
    wsi.webview_scroll_y = sy;
}

void InterLinksController::initialize_for_widgets(WorkStack &ws) {
    this->ws = &ws;
}

bool InterLinksController::uses_gl() const {
    return true;
}

void InterLinksController::gl_action() {
    if(this->ssw != nullptr) {
        this->connect_to_ssw();
        this->ssw->write();
    }
}

SceneWriter* InterLinksController::propose_scene_writer(const ControllerArgs &args,
                                                        SceneManager &sm) {
    const InterLinksControllerArgs& ilcargs = dynamic_cast<const InterLinksControllerArgs&>(args);

    this->ssw = new SimpleSceneWriter(ilcargs.from_id, sm);

    return this->ssw;
}

SceneWriter* InterLinksController::continue_scene_writer(const SceneWriter *sw) {
    const SimpleSceneWriter *other_ssw = dynamic_cast<const SimpleSceneWriter*>(sw);

    if(this->ssw != nullptr) {
        delete this->ssw;
    }
    this->ssw = new SimpleSceneWriter(*other_ssw);
    this->connect_to_ssw();

    return this->ssw;
}

void InterLinksController::connect_to_ssw() {
    connect(this->ssw, SIGNAL(show_inter_links_request(const uint32_t, const uint32_t)),
            this, SLOT(on_scene_inter_links_request(const uint32_t, const uint32_t)));
}

void InterLinksController::on_scene_inter_links_request(const uint32_t from_id, const uint32_t to_id) {
    QMap<QString, QVariant> args;
    args.insert("fromCodeID", from_id);
    args.insert("toCodeID", to_id);

    emit this->controller_change_request(CONTROLLER_INTER_CODE_LINKS, args);
}

InterLinksControllerArgs::InterLinksControllerArgs(uint32_t _from_id, uint32_t _to_id) :
  from_id(_from_id),
  to_id(_to_id)
{}

void InterLinksControllerArgs::from_js_object(const QMap<QString, QVariant> &obj) {
    if(obj.contains("fromCodeID")) {
        this->from_id = obj["fromCodeID"].toUInt();
    }
    if(obj.contains("toCodeID")) {
        this->to_id = obj["toCodeID"].toUInt();
    }
}

void InterLinksControllerArgs::to_js_object(QMap<QString, QVariant> &args) const {
    args.insert("fromCodeID", QVariant(this->from_id));
    args.insert("toCodeID", QVariant(this->to_id));
}
