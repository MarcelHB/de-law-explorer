/* SPDX-License-Identifier: GPL-3.0 */

#include "de_law_explorer.h"
#include "ui_de_law_explorer.h"
#include "db/loaders/loader_base.h"
#include "controllers/index_controller.h"
#include "controllers/code_index_controller.h"
#include "components/feedbackdialog.h"
#include "components/aboutdialog.h"
#include "components/updater.h"

/*
 *
 * This class handles all actions of the main window.
 *
 */

#define MAJOR_VERSION 1000
#define MINOR_VERSION 150101

DeLawExplorer::DeLawExplorer(Database &_db, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DeLawExplorer),
    tw(nullptr),
    wv(nullptr),
    glw(nullptr),
    db(_db)
{
    ui->setupUi(this);
    qRegisterMetaType<Controller*>("Controller*");
    qRegisterMetaType<ControllerArgs*>("ControllerArgs*");

    this->configureMenu();
    this->initialize_db();
    this->initializeComponents();
}

DeLawExplorer::~DeLawExplorer() {
    delete this->ui;
    delete this->router;
}

size_t DeLawExplorer::major_version() {
    return MAJOR_VERSION;
}

size_t DeLawExplorer::minor_version() {
    return MINOR_VERSION;
}

/*
 *
 * Configure Menu
 *
 */

void DeLawExplorer::configureMenu() {
    setupDeLawExplorerMenuActions();
}

void DeLawExplorer::setupDeLawExplorerMenuActions() {
    /* Go to. */
    QMenu *goto_menu = new QMenu(tr("&Gehe Zu"), this);
    menuBar()->addMenu(goto_menu);
    QAction *a = new QAction(tr("Gesetze - Index"), this);
    goto_menu->addAction(a);
    connect(a, SIGNAL(triggered()),
            this, SLOT(goto_codes_index()));

    /* Feedback. */
    QMenu *contact_menu = new QMenu(tr("&Kontakt"), this);
    a = new QAction(tr("&Feedback/Fehler"), this);
    menuBar()->addMenu(contact_menu);
    contact_menu->addAction(a);
    connect(a, SIGNAL(triggered()),
            this, SLOT(on_feedback_clicked()));


    /* About, etc. */
    QMenu *DeLawExplorer_menu = new QMenu(tr("&Mehr"), this);
    menuBar()->addMenu(DeLawExplorer_menu);

    a = new QAction(tr("U&pdate"), this);
    connect(a, SIGNAL(triggered()), this, SLOT(on_update_clicked()));
    DeLawExplorer_menu->addAction(a);

    a = new QAction(tr("&Über DeLawExplorer ..."), this);
    connect(a, SIGNAL(triggered()), this, SLOT(aboutDeLawExplorer()));
    DeLawExplorer_menu->addAction(a);
}

void DeLawExplorer::on_feedback_clicked() {
    QString sys_info;

    sys_info.append("Version: ");
    sys_info.append(QString::number(MAJOR_VERSION));
    sys_info.append(".");
    sys_info.append(QString::number(MINOR_VERSION));
    sys_info.append("\nDatabase: ");
    sys_info.append(this->db.version());

    FeedbackDialog fd(this);
    fd.system_information(sys_info);
    fd.exec();
}

void DeLawExplorer::on_update_clicked() {
    Updater updater(major_version(), minor_version(), this->db.version(), this);
    updater.exec();
}

void DeLawExplorer::goto_codes_index() {
    /* Empty - default index char. */
    QMap<QString, QVariant> args;
    this->wv->execute_web_request(CONTROLLER_INDEX, args);
}

/*
 *
 * Action: DeLawExplorer -> About DeLawExplorer
 *
 */

void DeLawExplorer::aboutDeLawExplorer() {
    AboutDialog ad(this->db, this);
    ad.exec();
}

/*
 *
 * Action: DeLawExplorer -> Quit DeLawExplorer
 *
 */

void DeLawExplorer::quitDeLawExplorer() {
    qApp->quit();
}

/*
 *
 * Initialize Components
 *
 */

void DeLawExplorer::initializeComponents() {
    this->showMaximized();

    WorkStack *ws = this->findChild<WorkStack*>("historyList");
    this->wv = this->findChild<WebView*>("webView");
    this->tw = this->findChild<QTabWidget*>("tabWidget");
    this->glw = this->findChild<GLWidget*>("widget");

    connect(ws, SIGNAL(item_to_dispose(const WorkStackItem&)),
            this, SLOT(on_stack_item_disposed(const WorkStackItem&)));
    connect(ws, SIGNAL(item_selected(const WorkStackItem&)),
            this, SLOT(on_stack_item_selection(const WorkStackItem&)));
    connect(this, SIGNAL(replay_dispose_request(const QString&)),
            this->wv, SLOT(on_replay_dispose_request(const QString&)));
    connect(this->wv, SIGNAL(request_history_step(size_t)),
            ws, SLOT(go_back(size_t)));

    this->router = new Router(*ws, *this->wv);
    this->wv->initialize(*router, false);

    connect(this->router, SIGNAL(controller_changed(Controller*, const ControllerArgs*)),
            this, SLOT(on_primary_controller_changed(Controller*, const ControllerArgs*)));

    QueryBar *qb = this->findChild<QueryBar*>("searchBar");
    connect(qb, SIGNAL(has_suggestion(int, const ControllerArgs*)),
            this, SLOT(on_search_suggestion(int, const ControllerArgs*)));
}

void DeLawExplorer::initialize_db() {
    LoaderBase::default_db(this->db.handle());
}

void DeLawExplorer::on_search_suggestion(int ctrl_id, const ControllerArgs *args) {
    QMap<QString, QVariant> web_args;
    args->to_js_object(web_args);
    this->wv->execute_web_request(ctrl_id, web_args);
}

void DeLawExplorer::on_stack_item_disposed(const WorkStackItem &wsi) {
    emit this->replay_dispose_request(wsi.replay_id);
}

void DeLawExplorer::on_stack_item_selection(const WorkStackItem &wsi) {
    if(this->wv->is_request_necessary(wsi.replay_id)) {
        /* Full program, async. */
        this->wv->execute_stack_request(wsi, true);
    } else {
        /* We still need the router to do things around the WebView. */
        this->router->request_controller_softly(wsi);
    }
}

void DeLawExplorer::on_controller_change_request(const int ctrl_id, const QMap<QString, QVariant> &args) {
    this->tw->setCurrentIndex(0);
    this->wv->execute_web_request(ctrl_id, args);
}

void DeLawExplorer::on_primary_controller_changed(Controller *ctrl, const ControllerArgs *args) {
    bool uses_gl = ctrl->uses_gl();
    this->tw->setTabEnabled(1, uses_gl);

    connect(ctrl, SIGNAL(controller_change_request(const int, const QMap<QString, QVariant>&)),
            this, SLOT(on_controller_change_request(const int, const QMap<QString, QVariant>&)));

    if(uses_gl) {
        this->glw->launch_controller(*ctrl, *args);
    } else {
        this->glw->invalidate_scene();
    }
}
