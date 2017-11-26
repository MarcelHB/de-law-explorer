/* SPDX-License-Identifier: GPL-3.0 */

#include <QLabel>
#include <QThread>

#include "work_stack.h"
#include "controllers/controller_response.h"

WorkStack::WorkStack(QWidget *parent) :
  QListWidget(parent)
{
    this->setStyleSheet("QListWidget { border: 0; outline: none; }");
    this->con_elem_clicked = connect(this, SIGNAL(itemClicked(QListWidgetItem*)),
                                     this, SLOT(on_item_clicked(QListWidgetItem*)));
    qRegisterMetaType<WorkStackItem>("WorkStackItem");
}

WorkStack::~WorkStack() {
    disconnect(this->con_elem_clicked);
}

void WorkStack::clear() {
    this->popn(this->count());
}

void WorkStack::go_back(size_t n) {
    this->go_back_internal(n);
}

void WorkStack::push(WorkStackItem &item) {
    this->stack.push(item);

    WorkStackItem& top = this->stack.top();
    this->add_label(top);
}

WorkStackItem& WorkStack::top() {
    return this->stack.top();
}

void WorkStack::add_label(const WorkStackItem &item) {
    if(this->thread() != QThread::currentThread()) {
        QMetaObject::invokeMethod(this, "add_label_slot", Qt::QueuedConnection,
                                  Q_ARG(WorkStackItem, item));
    } else {
        QLabel *label = new QLabel(item.text, this);
        label->setStyleSheet("QLabel { background-color: " + item.bg_color_code + ";"
                             "color: #FFFFFF; padding-left: 5px; }");

        QListWidgetItem *witem = new QListWidgetItem(this);
        witem->setSizeHint(QSize(label->sizeHint().width(), 50));

        this->addItem(witem);
        this->setItemWidget(witem, label);
    }
}

void WorkStack::add_label_slot(const WorkStackItem &wsi) {
    this->add_label(wsi);
}

void WorkStack::on_item_clicked(QListWidgetItem*) {
    if(this->currentRow() + 1 != this->count()) {
        int num = this->count() - this->currentRow() - 1;
        this->go_back_internal(num);
    }
}

void WorkStack::go_back_internal(size_t n) {
    if(this->stack.size() > 1) {
        this->popn(n);

        if(this->stack.size() > 0) {
            emit this->item_selected(this->stack.top());
        }
    }
}

void WorkStack::popn(size_t n) {
    while(n > 0) {
        this->takeItem(this->count() - 1);

        WorkStackItem& wsi = this->stack.top();
        emit this->item_to_dispose(wsi);

        if(wsi.args != nullptr) {
            delete wsi.args;
        }
        this->stack.pop();

        --n;
    }
}
