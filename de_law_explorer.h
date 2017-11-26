/* SPDX-License-Identifier: GPL-3.0 */

#ifndef DE_LAW_EXPLORER_H
#define DE_LAW_EXPLORER_H

#include <QMainWindow>
#include <QMessageBox>
#include <QDebug>
#include <QVBoxLayout>
#include <QtSql>

#include "db/database.h"
#include "controllers/router.h"
#include "3D/gl_widget.h"

namespace Ui {
class DeLawExplorer;
}

class DeLawExplorer : public QMainWindow {
    Q_OBJECT

signals:
    void replay_dispose_request(const QString&);
public:
    explicit DeLawExplorer(Database&, QWidget *parent = 0);
    ~DeLawExplorer();

    static size_t major_version();
    static size_t minor_version();
private slots:
    void aboutDeLawExplorer();
    void quitDeLawExplorer();
    void goto_codes_index();
    void on_controller_change_request(const int, const QMap<QString, QVariant>&);
    void on_feedback_clicked();
    void on_stack_item_disposed(const WorkStackItem&);
    void on_stack_item_selection(const WorkStackItem&);
    void on_update_clicked();
    void on_primary_controller_changed(Controller*,
                                       const ControllerArgs*);
    void on_search_suggestion(int, const ControllerArgs*);

private:
    void configureMenu();
    void setupDeLawExplorerMenuActions();

    void initializeComponents();
    void initialize_db();

private:
    Ui::DeLawExplorer *ui;
    Router *router;
    QTabWidget *tw;
    WebView *wv;
    GLWidget *glw;
    Database &db;
};

#endif // DE_LAW_EXPLORER_H
