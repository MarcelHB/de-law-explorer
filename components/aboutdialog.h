/* SPDX-License-Identifier: GPL-3.0 */

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

#include "../db/database.h"

namespace Ui {
class AboutDialog;
}

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(Database&, QWidget *parent = 0);
    ~AboutDialog();

private:
    Ui::AboutDialog *ui;

    void initialize_controls(Database&);
    void load_copyrights();
};

#endif // ABOUTDIALOG_H
