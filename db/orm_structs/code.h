/* SPDX-License-Identifier: GPL-3.0 */

#ifndef DB_LAW_EXPLORER_CODE_H
#define DB_LAW_EXPLORER_CODE_H

#include <cstdint>
#include <QString>

struct Code {
    uint32_t id;
    QString name;
    QString short_name;
    uint32_t size;
};

#endif
