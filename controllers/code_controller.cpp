/* SPDX-License-Identifier: GPL-3.0 */

#include <sstream>

#include "code_controller.h"

#include "../db/loaders/code_loader.h"

bool CodeController::action(ControllerArgs &args, ControllerResponse &r,
                             const QString&, const bool, const bool) {
    CodeControllerArgs& cargs = dynamic_cast<CodeControllerArgs&>(args);

    CodeLoader cl(cargs.code_id);
    cl.load();

    Code c;

    if(!cl.next_row(c)) {
        r.bad_response("Not found.", 404);
        return true;
    }

    /* Appending of uint32_t won't work properly. */
    std::stringstream ss;
    ss << c.id;
    std::string uint = ss.str();

    QString json("{\"id\":");
    json.append(QString::fromStdString(uint));
    json.append(",\"name\":\"");
    json.append(c.name);
    json.append("\", \"short_name\":\"");
    json.append(c.short_name);
    json.append("\", \"size\":");

    ss.str("");
    ss << c.size;
    uint = ss.str();
    json.append(QString::fromStdString(uint));
    json.append("}");

    r.succesful_response(json);

    return true;
}

void CodeController::stack_action(const ControllerArgs&, const int) {
}

bool CodeController::uses_gl() const {
    return false;
}

void CodeControllerArgs::from_js_object(const QMap<QString, QVariant> &obj) {
    if(obj.contains("codeID")) {
        this->code_id = obj["codeID"].toUInt();
    }
}

void CodeControllerArgs::to_js_object(QMap<QString, QVariant> &args) const {
    args.insert("codeID", QVariant(this->code_id));
}
