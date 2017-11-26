/* SPDX-License-Identifier: GPL-3.0 */

#include <sstream>
#include <vector>

#include <QString>

#include "code_index_controller.h"
#include "inter_links_controller.h"
#include "../db/orm_structs/code_atom_node.h"
#include "../db/loaders/code_loader.h"
#include "../db/loaders/code_atom_root_loader.h"
#include "../db/loaders/code_atom_tree_path_loader.h"
#include "../serialization/code_atom_tree_json_serializer.h"

CodeIndexController::CodeIndexController() :
  ws(nullptr),
  ssw(nullptr)
{}

CodeIndexController::~CodeIndexController() {
    if(this->ssw != nullptr) {
        delete this->ssw;
    }
}

bool CodeIndexController::action(ControllerArgs &args, ControllerResponse &r,
                                const QString &replay_id, const bool takeover,
                                const bool already_stacked) {
    CodeIndexControllerArgs& cicargs = dynamic_cast<CodeIndexControllerArgs&>(args);

    CodeLoader cl(cicargs.code_id);
    cl.load();

    Code c;
    if(!cl.next_row(c)) {
        r.bad_response("Not found.", 404);
        return already_stacked;
    }

    CodeAtomRootLoader carl(cicargs.code_id);
    carl.load();

    CodeAtomNode root;
    if(!carl.next_row(root.code_atom)) {
        r.bad_response("Not found.", 404);
        return already_stacked;
    }

    QString json = QString::fromStdString(this->json_tree(root));
    r.succesful_response(json);

    if(takeover && !already_stacked) {
        this->to_stack(cicargs, c, replay_id);
    }

    return true;
}

std::string CodeIndexController::json_tree(CodeAtomNode &root) const {
    uint32_t depth = root.code_atom.treeDepth;

    if(depth > 0) {
        CodeAtomTreePathLoader pl(root.code_atom.id, depth);
        pl.load();

        std::vector<CodeAtom> list;
        while(pl.next_row(list)) {
            CodeAtomNode *current_node = &root;
            for(auto it = list.cbegin(); it != list.cend(); ++it) {
                const CodeAtom& ca = *it;

                auto pos = current_node->children.find(ca.id);
                if(pos != current_node->children.cend()) {
                    current_node = &(pos->second);
                } else {
                    auto end = current_node->children.end();
                    CodeAtomNode next_atom;
                    next_atom.code_atom = ca;
                    auto next_pos = current_node->children.insert(end, pair<uint32_t, CodeAtomNode>(ca.id, next_atom));
                    current_node = &(next_pos->second);
                }
            }

            list.clear();
        }
    }

    QMap<uint32_t, vector<CodeAtomLink>> linkMap;

    CodeAtomTreeJSONSerializer ser(root, linkMap);
    ser.serialize();
    std::stringstream &ss = ser.stringstream();
    std::string str = ss.str();

    return str;
}

void CodeIndexController::initialize_for_widgets(WorkStack &ws) {
    this->ws = &ws;
}

void CodeIndexController::to_stack(CodeIndexControllerArgs &cicargs, const Code &c,
                                   const QString &replay_id) {
    if(this->ws != nullptr) {
        WorkStackItem wsi = {
            c.short_name,
            QString("#DEA5A4"),
            CONTROLLER_CODE_INDEX,
            &cicargs,
            0,
            QString(replay_id)
        };
        this->ws->push(wsi);
    }
}

void CodeIndexController::stack_action(const ControllerArgs&, const int sy) {
    WorkStackItem& wsi = this->ws->top();
    wsi.webview_scroll_y = sy;
}

bool CodeIndexController::uses_gl() const {
    return true;
}

void CodeIndexController::gl_action() {
    if(this->ssw != nullptr) {
        this->connect_to_ssw();
        this->ssw->write();
    }
}

SceneWriter* CodeIndexController::propose_scene_writer(const ControllerArgs &args, SceneManager &sm) {
    const CodeIndexControllerArgs& cicargs = dynamic_cast<const CodeIndexControllerArgs&>(args);

    this->ssw = new SimpleSceneWriter(cicargs.code_id, sm);

    return this->ssw;
}

SceneWriter* CodeIndexController::continue_scene_writer(const SceneWriter *sw) {
    const SimpleSceneWriter *other_ssw = dynamic_cast<const SimpleSceneWriter*>(sw);

    if(this->ssw != nullptr) {
        delete this->ssw;
    }
    this->ssw = new SimpleSceneWriter(*other_ssw);
    this->connect_to_ssw();

    return this->ssw;
}

void CodeIndexController::connect_to_ssw() {
    connect(this->ssw, SIGNAL(show_inter_links_request(const uint32_t, const uint32_t)),
            this, SLOT(on_scene_inter_links_request(const uint32_t, const uint32_t)));
}

void CodeIndexController::on_scene_inter_links_request(const uint32_t from_id, const uint32_t to_id) {
    QMap<QString, QVariant> args;
    args.insert("fromCodeID", from_id);
    args.insert("toCodeID", to_id);

    emit this->controller_change_request(CONTROLLER_INTER_CODE_LINKS, args);
}

void CodeIndexControllerArgs::from_js_object(const QMap<QString, QVariant> &obj) {
    if(obj.contains("codeID")) {
        this->code_id = obj["codeID"].toUInt();
    }
}

void CodeIndexControllerArgs::to_js_object(QMap<QString, QVariant> &args) const {
    args.insert("codeID", QVariant(this->code_id));
}
