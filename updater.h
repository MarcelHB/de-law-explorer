/* SPDX-License-Identifier: GPL-3.0 */

#ifndef UPDATER_H
#define UPDATER_H

#include <QDialog>

namespace Ui {
class Updater;
}

class Updater : public QDialog
{
    Q_OBJECT

public:
    explicit Updater(QWidget *parent = 0);
    ~Updater();

private:
    Ui::Updater *ui;
};

#endif // UPDATER_H
