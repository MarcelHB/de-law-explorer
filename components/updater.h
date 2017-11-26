/* SPDX-License-Identifier: GPL-3.0 */

#ifndef UPDATER_H
#define UPDATER_H

#include <cstdint>

#include <QDialog>
#include <QProgressBar>
#include <QTextEdit>

#include "updater_tools/update_query.h"

namespace Ui {
class Updater;
}

class Updater : public QDialog {
  Q_OBJECT
  public:
    explicit Updater(size_t, size_t, uint32_t, QWidget *parent = 0);
    ~Updater();
  private slots:
    void on_query_data_received(const UpdateQueryData&);
    void on_query_failed(UpdateQueryError);
    void on_start_update();

  private:
    Ui::Updater *ui;
    QProgressBar *progress_bar;
    QTextEdit *progress_text;
    UpdateQuery *query_thread;
    size_t version_major;
    size_t version_minor;
    uint32_t db_version;
    bool can_update;
    UpdateQueryData update_data;

    QMetaObject::Connection con_query_data_received;
    QMetaObject::Connection con_query_failed;

    void allow_start_update(const UpdateQueryData&);
    void connect_query_events();
    void disconnect_query_events();
    void initialize_controls();
    void show_complete_progress();
};

#endif // UPDATER_H
