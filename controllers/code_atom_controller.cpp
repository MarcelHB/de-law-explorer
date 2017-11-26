/* SPDX-License-Identifier: GPL-3.0 */

#include <sstream>
#include <vector>

#include <QString>

#include "code_atom_controller.h"
#include "inter_links_controller.h"
#include "../db/atom_type_enum.h"
#include "../db/orm_structs/code_atom_node.h"
#include "../db/loaders/code_loader.h"
#include "../db/loaders/code_atom_loader.h"
#include "../db/loaders/code_atom_link_loader.h"
#include "../db/loaders/code_atom_tree_path_loader.h"
#include "../db/loaders/code_atom_tree_path_reverse_loader.h"
#include "../serialization/code_atom_tree_json_serializer.h"

CodeAtomController::CodeAtomController() :
  ws(nullptr),
  ssw(nullptr)
{}

CodeAtomController::~CodeAtomController() {
    if(this->ssw != nullptr) {
        delete this->ssw;
    }
}

bool CodeAtomController::action(ControllerArgs &args, ControllerResponse &r,
                                const QString &replay_id, const bool takeover,
                                const bool already_stacked) {
    CodeAtomControllerArgs& cacargs = dynamic_cast<CodeAtomControllerArgs&>(args);

    CodeAtomLoader cal(cacargs.code_atom_id);
    cal.load();

    CodeAtomNode root;
    if(!cal.next_row(root.code_atom))
        return already_stacked;

    if(root.code_atom.atomType > TYPE_PARAGRAPH) {
        CodeAtomTreePathReverseLoader rpl(cacargs.code_atom_id, root.code_atom.depth - 1);
        rpl.load();

        vector<CodeAtom> list;
        while(rpl.next_row(list));

        for(size_t i = 0; i < list.size(); ++i) {
            const CodeAtom ca = list.at(i);
            if(ca.atomType == TYPE_PARAGRAPH) {
                CodeAtomLoader cal(ca.id);
                cal.load();

                if(!cal.next_row(root.code_atom))
                    return already_stacked;

                break;
            }
        }
    }

    QString json = this->json_tree(root);
    r.succesful_response(json);

    if(takeover && !already_stacked) {
        this->to_stack(cacargs, root.code_atom, replay_id);
    }

    return true;
}

QString CodeAtomController::json_tree(CodeAtomNode &root) const {
    uint32_t depth = root.code_atom.treeDepth;

    vector<int> atomsWithLinks;
    if(root.code_atom.atomType == TYPE_SATZ)
        atomsWithLinks.push_back(root.code_atom.id);

    if(depth > 0) {
        CodeAtomTreePathLoader pl(root.code_atom.id, depth);
        pl.load();

        vector<CodeAtom> list;
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

                    if(ca.atomType == TYPE_SATZ)
                        atomsWithLinks.push_back(ca.id);
                }
            }

            list.clear();
        }
    }

    QMap<uint32_t, vector<CodeAtomLink>> linkMap;
    if(atomsWithLinks.size() > 0) {
        CodeAtomLinkLoader ll(atomsWithLinks);
        ll.load();
        while(ll.next_row(linkMap));
    }

    CodeAtomTreeJSONSerializer ser(root, linkMap);
    ser.serialize();
    std::stringstream &ss = ser.stringstream();
    std::string str = ss.str();

    return QString::fromStdString(str);
}

void CodeAtomController::initialize_for_widgets(WorkStack &ws) {
    this->ws = &ws;
}

void CodeAtomController::to_stack(CodeAtomControllerArgs &cacargs, const CodeAtom &ca,
                                  const QString &replay_id) {
    if(this->ws != nullptr) {
        WorkStackItem wsi = {
            this->stack_name(ca),
            QString(this->stack_color(ca)),
            CONTROLLER_CODE_ATOM,
            &cacargs,
            0,
            QString(replay_id)
        };
        this->ws->push(wsi);
    }
}

const char* CodeAtomController::stack_color(const CodeAtom &ca) const {
    switch(ca.atomType) {
        case TYPE_MASTER:
            return "#DEA5A4";
        case TYPE_ABSCHNITT:
            return "#F49AC2";
        case TYPE_PARAGRAPH:
            return "#779ECB";
        case TYPE_ARTIKEL:
            return "#77DD77";
        case TYPE_ABSATZ:
            return "#B39EB5";
        case TYPE_SATZ:
            return "#FFD1DC";
        case TYPE_NUMMER:
            return "#AEC6CF";
        case TYPE_ANHANG:
            return "#555753";
        case TYPE_FUSSNOTE:
            return "#FF6961";
    }

    return "#7D7D7D";
}

QString CodeAtomController::stack_name(const CodeAtom &ca) const {
    CodeLoader cl(ca.codeId);
    cl.load();

    Code c;
    cl.next_row(c);

    QString name("");

    switch(ca.atomType) {
        case TYPE_PARAGRAPH:
            name.append("§ ");
            name.append(ca.key);
            name.append(" ");
            name.append(c.short_name);
            break;
        case TYPE_ARTIKEL:
            name.append("Art ");
            name.append(ca.key);
            name.append(" ");
            name.append(c.short_name);
            break;
        case TYPE_ANHANG:
            name.append("Anlage ");
            name.append(ca.key);
            name.append(" ");
            name.append(c.short_name);
            break;
        default:
            break;
    }

    return name;
}

void CodeAtomController::stack_action(const ControllerArgs&, const int sy) {
    WorkStackItem& wsi = this->ws->top();
    wsi.webview_scroll_y = sy;
    // TODO: take inline history into account
}

bool CodeAtomController::uses_gl() const {
    return true;
}

void CodeAtomController::gl_action() {
    if(this->ssw != nullptr) {
        this->connect_to_ssw();
        this->ssw->write();
    }
}

SceneWriter* CodeAtomController::propose_scene_writer(const ControllerArgs &args, SceneManager &sm) {
    const CodeAtomControllerArgs& cacargs = dynamic_cast<const CodeAtomControllerArgs&>(args);

    CodeAtom ca;
    CodeAtomLoader cal(cacargs.code_atom_id);
    cal.load();
    cal.next_row(ca);

    this->ssw = new SimpleSceneWriter(ca.codeId, sm);
    return this->ssw;
}

SceneWriter* CodeAtomController::continue_scene_writer(const SceneWriter *sw) {
    const SimpleSceneWriter *other_ssw = dynamic_cast<const SimpleSceneWriter*>(sw);

    if(this->ssw != nullptr) {
        delete this->ssw;
    }
    this->ssw = new SimpleSceneWriter(*other_ssw);
    this->connect_to_ssw();

    return this->ssw;
}

void CodeAtomController::connect_to_ssw() {
    connect(this->ssw, SIGNAL(show_inter_links_request(const uint32_t, const uint32_t)),
            this, SLOT(on_scene_inter_links_request(const uint32_t, const uint32_t)));
}

void CodeAtomController::on_scene_inter_links_request(const uint32_t from_id, const uint32_t to_id) {
    QMap<QString, QVariant> args;
    args.insert("fromCodeID", from_id);
    args.insert("toCodeID", to_id);

    emit this->controller_change_request(CONTROLLER_INTER_CODE_LINKS, args);
}

void CodeAtomControllerArgs::from_js_object(const QMap<QString, QVariant> &obj) {
    if(obj.contains("codeAtomID")) {
        this->code_atom_id = obj["codeAtomID"].toUInt();
    }
}

void CodeAtomControllerArgs::to_js_object(QMap<QString, QVariant> &args) const {
    args.insert("codeAtomID", QVariant(this->code_atom_id));
}
