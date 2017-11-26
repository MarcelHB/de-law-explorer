/* SPDX-License-Identifier: GPL-3.0 */

#include "updater.h"
#include "ui_updater.h"

Updater::Updater(size_t major, size_t minor, uint32_t db,
                 QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Updater),
    version_major(major),
    version_minor(minor),
    db_version(db)
{
    ui->setupUi(this);
}

Updater::~Updater()
{
    delete ui;
}
