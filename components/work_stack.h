/* SPDX-License-Identifier: GPL-3.0 */

#ifndef DE_LAW_EXPLORER_WORK_STACK_H
#define DE_LAW_EXPLORER_WORK_STACK_H

#include <stack>
#include <QString>
#include <QListWidget>

class WorkStack;
struct WorkStackItem;

#include "controllers/controller.h"

struct WorkStackItem {
    QString text;
    QString bg_color_code;
    size_t ctrl_id;
    ControllerArgs *args;
    int webview_scroll_y;
    QString replay_id;
};

class WorkStack : public QListWidget {
  Q_OBJECT

  signals:
    void item_to_dispose(const WorkStackItem&);
    void item_selected(const WorkStackItem&);
  public:
    WorkStack(QWidget*);
    ~WorkStack();

    void clear();
    void push(WorkStackItem&);
    WorkStackItem& top();
  public slots:
    void add_label_slot(const WorkStackItem&);
    void go_back(size_t);
  private:
    std::stack<WorkStackItem> stack;
    QMetaObject::Connection con_elem_clicked;

    void add_label(const WorkStackItem&);
    void go_back_internal(size_t);
    void popn(size_t);
  private slots:
    void on_item_clicked(QListWidgetItem*);
};

#endif
