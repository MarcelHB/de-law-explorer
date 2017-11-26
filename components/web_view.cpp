/* SPDX-License-Identifier: GPL-3.0 */

#include <QApplication>
#include <QFile>
#include <QKeyEvent>
#include <QMouseEvent>

#include "web_view.h"

WebView::WebView(QWidget *parent) :
  QWebView(parent),
  with_inspector(false),
  inspector(nullptr),
  dispatcher(nullptr)
{}

WebView::~WebView() {
    disconnect(this->con_wv_loaded);

    if(this->dispatcher != nullptr) {
        delete this->dispatcher;
    }

    if(this->with_inspector) {
        delete this->inspector;
    }
}

void WebView::execute_stack_request(const WorkStackItem &wsi, const bool stacked) {
    if(this->dispatcher != nullptr) {
        this->dispatcher->do_request(wsi, stacked);
    }
}

void WebView::execute_web_request(const int ctrlID, const QMap<QString, QVariant>& args) {
    if(this->dispatcher != nullptr) {
        this->dispatcher->do_web_request(ctrlID, args);
    }
}

void WebView::initialize(Router &r, bool debugging) {
    this->with_inspector = debugging;
    this->dispatcher = new WebDispatcher(r);

    this->main_frame().setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOn);

    this->con_wv_loaded = connect(this, SIGNAL(loadFinished(bool)),
                                  this, SLOT(on_webview_loaded(bool)));
    this->load(QUrl("qrc:/assets/html/index.html"));
}

/* As we need a return value from JS, we must do it *here* and like this.*/
bool WebView::is_request_necessary(const QString &request_id) {
    QString call("window.checkIfRequestNeeded(\"");
    call.append(request_id);
    call.append("\");");

    QVariant result = this->main_frame().evaluateJavaScript(call);
    return result.toBool();
}

int WebView::scroll_height() {
    return this->main_frame().scrollBarValue(Qt::Vertical);
}

void WebView::initialize_controls() {
    if(this->with_inspector) {
        this->page()->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
        this->inspector = new QWebInspector;
        this->inspector->setPage(this->page());
        this->inspector->show();
    }
}

void WebView::keyPressEvent(QKeyEvent *e) {
    /* We must pass-through any input to use input-fields. */
    QWebView::keyPressEvent(e);
    int k = e->key();

    switch(k) {
    case Qt::Key_Backspace:
        emit this->request_history_step(1);
        break;
    case Qt::Key_T:
        if(this->with_inspector) {
            this->inspector->setVisible(!inspector->isVisible());
        }
        break;
    case Qt::Key_F:
        if(QApplication::keyboardModifiers() & Qt::ControlModifier) {
            QMetaObject::invokeMethod(this, "on_inline_search_start",
                                      Qt::QueuedConnection);
        }
        break;
    default:
        break;
    }
}

void WebView::mousePressEvent(QMouseEvent *e) {
    QWebView::mousePressEvent(e);
    Qt::MouseButton k = e->button();

    switch(k) {
      case Qt::BackButton:
        emit this->request_history_step(1);
        break;
      default:
        break;
    }
}

QWebFrame& WebView::main_frame() {
    return *(this->page()->mainFrame());
}

void WebView::on_replay_dispose_request(const QString &req_id) {
    if(this->dispatcher != nullptr) {
        this->dispatcher->request_replay_disposal(req_id);
    }
}

void WebView::on_webview_loaded(bool) {
    QFile file;
    file.setFileName(":/assets/javascripts/qt.js");
    file.open(QIODevice::ReadOnly);
    QByteArray bridge_js = file.readAll();
    file.close();

    this->main_frame().addToJavaScriptWindowObject("WebDispatcher", this->dispatcher);
    this->main_frame().evaluateJavaScript(bridge_js);

    this->initialize_controls();

    this->dispatcher->indicate_ready();
}
