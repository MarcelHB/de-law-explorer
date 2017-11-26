/* SPDX-License-Identifier: GPL-3.0 */

#include "code_back_link_controller.h"

#include <sstream>
#include <QString>

#include "../db/loaders/code_atom_back_link_loader.h"
#include "../serialization/code_back_link_json_fly_serializer.h"

bool CodeBackLinkController::action(ControllerArgs &args, ControllerResponse &r,
                                    const QString&, const bool, const bool) {
    CodeBackLinkControllerArgs& cblargs = dynamic_cast<CodeBackLinkControllerArgs&>(args);

    CodeAtomBackLinkLoader cbl(cblargs.code_atom_id);
    cbl.load();

    CodeBackLinkJSONFlySerializer fs;
    CodeAtomLink codeAtomLink;
    int cnt = 0;
    while(cbl.next_row(codeAtomLink)) {
        fs.code(codeAtomLink);
        cnt++;
    }
    if(cnt==0)
        fs.start();

    fs.end();

    stringstream &ss = fs.stringstream();
    string str = ss.str();
    QString json = QString::fromStdString(str);

    r.succesful_response(json);

    return false;
}

void CodeBackLinkController::stack_action(const ControllerArgs&, const int) {
}

bool CodeBackLinkController::uses_gl() const {
    return false;
}

void CodeBackLinkControllerArgs::from_js_object(const QMap<QString, QVariant> &obj) {
    if(obj.contains("codeAtomID")) {
        this->code_atom_id = obj["codeAtomID"].toUInt();
    }
}

void CodeBackLinkControllerArgs::to_js_object(QMap<QString, QVariant> &args) const {
    args.insert("codeAtomID", QVariant(this->code_atom_id));
}
