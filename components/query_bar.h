/* SPDX-License-Identifier: GPL-3.0 */

#ifndef QUERY_BAR_H
#define QUERY_BAR_H

#include <QCompleter>
#include <QLineEdit>

#include "../controllers/controller.h"

class QueryBar : public QLineEdit {
  Q_OBJECT
  public:
    QueryBar(QWidget*);
    ~QueryBar();
  private:
    QCompleter *completer;
    QMetaObject::Connection con_query_request;
    enum { EMPTY, FILLING, CLICKED } state;

    void initialize();

    static QString default_string;
    static QString active_text_style;
    static QString passive_text_style;
  protected:
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *e);
  signals:
    void has_suggestion(int, const ControllerArgs*);
  private slots:
    void on_enter();
};

#endif // QUERY_BAR_H
