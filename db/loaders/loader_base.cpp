/* SPDX-License-Identifier: GPL-3.0 */

#include "loader_base.h"

void LoaderBase::default_db(QSqlDatabase &db) {
    _default_db = &db;
}

QSqlDatabase* LoaderBase::default_db() {
    return _default_db;
}

QSqlDatabase* LoaderBase::_default_db = NULL;

