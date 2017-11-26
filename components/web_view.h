/* SPDX-License-Identifier: GPL-3.0 */

#ifndef WEB_VIEW_H
#define WEB_VIEW_H

#include <cstdint>

#include <QLineEdit>
#include <QPushButton>
#include <QWidget>
#include <QWebView>
#include <QWebFrame>
#include <QWebInspector>

class WebView;

#include "web_dispatcher.h"

class WebView : public QWebView {
  Q_OBJECT
  public:
    WebView(QWidget*);
    ~WebView();

    void execute_stack_request(const WorkStackItem&, const bool);
    void execute_web_request(const int, const QMap<QString, QVariant>&);
    void initialize(Router&, bool dbg=false);
    bool is_request_necessary(const QString&);
    int scroll_height();
  public slots:
    void on_replay_dispose_request(const QString&);
  protected:
    void keyPressEvent(QKeyEvent *e);
    void mousePressEvent(QMouseEvent *e);
  private:
    bool with_inspector;
    QMetaObject::Connection con_wv_loaded;
    QWebInspector *inspector;
    WebDispatcher *dispatcher;

    void initialize_controls();
    QWebFrame& main_frame();
  signals:
    void request_history_step(size_t);
  private slots:
    void on_webview_loaded(bool);
};

#endif // WEB_VIEW_H
