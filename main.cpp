/* SPDX-License-Identifier: GPL-3.0 */

#include "de_law_explorer.h"
#include "components/termswindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    Database db("test.db");

    if(!db.open()) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText(db.open_error());
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        // TODO: better termination logic!
        throw new std::exception;
    }

    DeLawExplorer w(db);
    w.show();

    TermsWindow tw(db, &w);
    tw.show();

    return a.exec();
}
