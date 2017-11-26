/* SPDX-License-Identifier: GPL-3.0 */

#include <QDesktopWidget>
#include <QFile>
#include <QTextStream>

#include "termswindow.h"
#include "ui_termswindow.h"
#include "aboutdialog.h"

TermsWindow::TermsWindow(Database &_db, QWidget *parent) :
    QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint), /* No close/min/max btns. */
    ui(new Ui::TermsWindow),
    db(_db)
{
    ui->setupUi(this);
    this->initialize_controls();
}

TermsWindow::~TermsWindow()
{
    delete ui;
}

void TermsWindow::initialize_controls() {
    this->setFixedSize(this->size());

    /* Screen centering. */
    QRect scr = QApplication::desktop()->screenGeometry();
    this->move(scr.center() - rect().center());

    QPushButton *ok_btn = this->findChild<QPushButton*>("pushButton_2");
    connect(ok_btn, SIGNAL(clicked()),
            this, SLOT(close()));

    QPushButton *about_btn = this->findChild<QPushButton*>("pushButton");
    connect(about_btn, SIGNAL(clicked()),
            this, SLOT(open_about()));

    this->load_terms();
}

void TermsWindow::open_about() {
    AboutDialog ad(this->db, this);
    ad.exec();
}

void TermsWindow::load_terms() {
    QFile res(":/assets/text/terms.txt");
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

