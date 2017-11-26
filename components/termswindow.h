/* SPDX-License-Identifier: GPL-3.0 */

#ifndef TERMSWINDOW_H
#define TERMSWINDOW_H

#include <QDialog>

#include "../db/database.h"

namespace Ui {
class TermsWindow;
}

class TermsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit TermsWindow(Database&, QWidget *parent = 0);
    ~TermsWindow();

private:
    Ui::TermsWindow *ui;
    Database &db;

    void initialize_controls();
    void load_terms();

  private slots:
    void open_about();
};

#endif // TERMSWINDOW_H
