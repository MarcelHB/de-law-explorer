/* SPDX-License-Identifier: GPL-3.0 */

#include "query_bar.h"

#include "query_tools/search_performer.h"

QString QueryBar::default_string =
        QString("Tippen Sie z.B. \"§ 573 BGB\", \"Strafgesetzbuch\", \"UStG\" oder \"ZPO:StPO\" ein");
QString QueryBar::active_text_style = QString("color: black");
QString QueryBar::passive_text_style = QString("color: gray; font-style: italic");

QueryBar::QueryBar(QWidget *parent) :
  QLineEdit(parent),
  completer(nullptr),
  state(EMPTY)
{
    this->initialize();
}

QueryBar::~QueryBar() {
    disconnect(this->con_query_request);
    delete this->completer;
}

void QueryBar::initialize() {
    this->con_query_request = connect(this, SIGNAL(returnPressed()),
                                      this, SLOT(on_enter()));
    this->setText(tr(&default_string.toStdString().c_str()[0]));
    this->setStyleSheet(passive_text_style);
}

void QueryBar::focusInEvent(QFocusEvent*) {
    if(this->state == EMPTY || this->state == CLICKED) {
        this->setText("");
        this->setStyleSheet(active_text_style);
        this->state = EMPTY;
    }
}

void QueryBar::focusOutEvent(QFocusEvent*) {
    if(this->state == EMPTY) {
        QString str = this->text();
        if(str.length() > 0) {
            this->state = FILLING;
        } else {
            this->setText(tr(&default_string.toStdString().c_str()[0]));
            this->setStyleSheet(passive_text_style);
        }
    }
}

void QueryBar::on_enter() {
    QString str = this->text();

    if(str.length() > 0) {
        SearchPerformer sp(this->text());
        SearchSuggestion sg = sp.run();

        emit this->has_suggestion(sg.ctrl_id, sg.args);
        this->state = CLICKED;
        /* Not that good ... must be read blocking on async. */
        delete sg.args;
    }
}
