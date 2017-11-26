/* SPDX-License-Identifier: GPL-3.0 */

#ifndef CODE_ATOM_BATCH_LOOKUP_CONTROLLER_H
#define CODE_ATOM_BATCH_LOOKUP_CONTROLLER_H

#include <cstdint>
#include <QVector>

#include "controller.h"

#define CONTROLLER_ATOM_BATCH_LOOKUP    8

class CodeAtomBatchLookupControllerArgs : public ControllerArgs {
  public:
    CodeAtomBatchLookupControllerArgs() {}
    CodeAtomBatchLookupControllerArgs(const QVector<uint32_t>&);

    void from_js_object(const QMap<QString, QVariant>&);
    void to_js_object(QMap<QString, QVariant>&) const;

    QVector<uint32_t> ids;
};

class CodeAtomBatchLookupController : public Controller {
  Q_OBJECT
  public:
    bool action(ControllerArgs&, ControllerResponse&, const QString&, const bool, const bool);
    void stack_action(const ControllerArgs&, const int);
    bool uses_gl() const;
};

#endif // CODE_ATOM_BATCH_LOOKUP_CONTROLLER_H
