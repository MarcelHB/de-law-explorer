/* SPDX-License-Identifier: GPL-3.0 */

#include <QFile>

#include "../de_law_explorer.h"
#include "aboutdialog.h"
#include "ui_aboutdialog.h"

AboutDialog::AboutDialog(Database &db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    this->initialize_controls(db);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::initialize_controls(Database &db) {
    this->setFixedSize(this->size());
    this->setWindowTitle(tr("About DeLawExplorer"));

    QLineEdit *version_line = this->findChild<QLineEdit*>("lineEdit");

    QString app_version;
    app_version.append(QString::number(DeLawExplorer::major_version()));
    app_version.append(".");
    app_version.append(QString::number(DeLawExplorer::minor_version()));
    version_line->setText(app_version);

    QLineEdit *db_version_line = this->findChild<QLineEdit*>("lineEdit_2");
    db_version_line->setText(QString::number(db.version()));

    QLabel *link_label = this->findChild<QLabel*>("label_3");
    link_label->setOpenExternalLinks(true);

    QPushButton *close_btn = this->findChild<QPushButton*>("pushButton");
    connect(close_btn, SIGNAL(clicked()),
            this, SLOT(close()));

    this->load_copyrights();
}


void AboutDialog::load_copyrights() {
    QFile res(":/assets/text/copyrights.txt");
    QString buffer;
    res.open(QFile::ReadOnly | QFile::Text);

    QTextStream in(&res);
    while (!in.atEnd()) {
       buffer.append(in.readLine());
       buffer.append("<br />");
    }

    QTextBrowser *text_view = this->findChild<QTextBrowser*>("textBrowser");
    text_view->setText(buffer);

    res.close();
}
