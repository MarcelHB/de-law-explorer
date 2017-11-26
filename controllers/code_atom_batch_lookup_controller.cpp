/* SPDX-License-Identifier: GPL-3.0 */

#include "code_atom_batch_lookup_controller.h"

#include <sstream>

#include "../db/loaders/code_atom_max_depth_loader.h"
#include "../db/loaders/code_atom_tree_reverse_path_by_ids_loader.h"
#include "../serialization/code_batch_lookup_json_serializer.h"

bool CodeAtomBatchLookupController::action(ControllerArgs &args, ControllerResponse &r,
                                           const QString &requestID, const bool, const bool) {
    CodeAtomBatchLookupControllerArgs& cablcargs = dynamic_cast<CodeAtomBatchLookupControllerArgs&>(args);

    CodeAtomMaxDepthLoader dl(cablcargs.ids);
    dl.load();

    uint32_t depth = 0;
    if(!dl.next_row(depth)) {
        return false;
    }

    CodeAtomTreeReversePathByIDsLoader revl(cablcargs.ids, depth);
    revl.load();

    CodeBatchLookupJSONSerializer ser;
    std::vector<CodeAtom> ca_list;

    while(revl.next_row(ca_list)) {
        ser.path(ca_list);
        ca_list.clear();
    }
    ser.end();

    std::stringstream &ss = ser.stringstream();
    std::string str = ss.str();
    QString json = QString::fromStdString(str);

    r.succesful_response(json);

    return false;
}

void CodeAtomBatchLookupController::stack_action(const ControllerArgs &, const int) {
    /* Nothing. */
}

bool CodeAtomBatchLookupController::uses_gl() const {
    return false;
}

CodeAtomBatchLookupControllerArgs::CodeAtomBatchLookupControllerArgs(const QVector<uint32_t> &_ids) :
  ids(_ids)
{}

void CodeAtomBatchLookupControllerArgs::from_js_object(const QMap<QString, QVariant> &obj) {
    if(obj.contains("codeAtomIDs")) {
        QString ids_string = obj["codeAtomIDs"].toString();
        QStringList ids_string_list = ids_string.split(",", QString::SkipEmptyParts);

        for(auto it = ids_string_list.cbegin(); it != ids_string_list.cend(); ++it) {
            QString id_string = *it;
            this->ids.append(static_cast<uint32_t>(id_string.toUInt()));
        }
    }
}

void CodeAtomBatchLookupControllerArgs::to_js_object(QMap<QString, QVariant> &obj) const {
    QStringList ids_string;

    for(auto it = this->ids.cbegin(); it != this->ids.cend(); ++it) {
        uint32_t id = *it;
        QString id_string = QString::number(id);
        ids_string.append(id_string);
    }

    obj["codeAtomIDs"] = ids_string.join(",");
}
