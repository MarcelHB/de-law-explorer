/* SPDX-License-Identifier: GPL-3.0 */

#include "updater.h"
#include "ui_updater.h"

Updater::Updater(size_t major, size_t minor, uint32_t db,
                 QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Updater),
    progress_bar(nullptr),
    progress_text(nullptr),
    query_thread(nullptr),
    version_major(major),
    version_minor(minor),
    db_version(db),
    can_update(false)
{
    ui->setupUi(this);
    this->initialize_controls();

    this->query_thread = new UpdateQuery(this->version_major, this->version_minor, this->db_version);
    this->connect_query_events();
    this->query_thread->start();

    QString initial_string(tr("Suche nach Updates ..."));
    initial_string.append("\n");
    this->progress_text->setText(initial_string);
}

Updater::~Updater() {
    this->disconnect_query_events();

    if(this->query_thread != nullptr) {
        this->query_thread->abort();
        delete this->query_thread;
    }

    delete ui;
}

void Updater::on_query_data_received(const UpdateQueryData &data) {
    this->show_complete_progress();
    this->disconnect_query_events();

    bool any_update = data.has_application_update || data.has_application_update;
    QString progress_string = this->progress_text->toPlainText();

    if(any_update) {
        if(data.has_application_update) {
            progress_string.append(tr("Update gefunden: Programmversion "));
            progress_string.append(QString::number(data.version_major));
            progress_string.append(".");
            progress_string.append(QString::number(data.version_minor));
            progress_string.append("\n");
        }
        if(data.has_database_update) {
            progress_string.append(tr("Update gefunden: Datenbankversion "));
            progress_string.append(QString::number(data.database_version));
            progress_string.append("\n");
        }
        this->allow_start_update(data);
    } else {
        progress_string.append(tr("Keine Updates gefunden."));
        progress_string.append("\n");
    }

    this->progress_text->setText(progress_string);
}

void Updater::on_query_failed(UpdateQueryError error) {
    this->show_complete_progress();
    this->disconnect_query_events();

    QString progress_string = this->progress_text->toPlainText();
    progress_string.append(tr("Fehler: "));

    switch(error) {
        case NETWORK_ERROR:
            progress_string.append(tr("Netzwerkfehler oder Server nicht erreichbar."));
            break;
        case OBJECT_ERROR:
            progress_string.append(tr("Keine Daten für das Update gefunden."));
            break;
        case PARSER_ERROR:
            progress_string.append(tr("Unbekanntes Update-Protokoll."));
            break;
        default:
            progress_string.append(tr("Unbekannte Ursache."));
            break;
    }

    this->progress_text->setText(progress_string);
}

void Updater::on_start_update() {
    if(this->can_update) {
        this->can_update = false;
        QPushButton *start_update_btn = this->findChild<QPushButton*>("pushButton_2");
        start_update_btn->setEnabled(false);

        QPushButton *ok_btn = this->findChild<QPushButton*>("pushButton");
        ok_btn->setText(tr("Abbrechen"));
    }
}

void Updater::disconnect_query_events() {
    this->disconnect(this->con_query_data_received);
    this->disconnect(this->con_query_failed);
}

void Updater::allow_start_update(const UpdateQueryData &data) {
    this->can_update = true;
    this->update_data = data;

    QPushButton *start_update_btn = this->findChild<QPushButton*>("pushButton_2");
    start_update_btn->setEnabled(true);

    connect(start_update_btn, SIGNAL(clicked()),
            this, SLOT(on_start_update()));
}

void Updater::connect_query_events() {
    this->con_query_data_received = connect(this->query_thread, SIGNAL(update_data_received(const UpdateQueryData&)),
                                            this, SLOT(on_query_data_received(const UpdateQueryData&)));
    this->con_query_failed = connect(this->query_thread, SIGNAL(update_impossible(UpdateQueryError)),
                                          this, SLOT(on_query_failed(UpdateQueryError)));
}

void Updater::initialize_controls() {
    this->setFixedSize(this->size());

    QPushButton *ok_btn = this->findChild<QPushButton*>("pushButton");
    connect(ok_btn, SIGNAL(clicked()),
            this, SLOT(close()));

    this->progress_bar = this->findChild<QProgressBar*>("progressBar");
    this->progress_text = this->findChild<QTextEdit*>("textEdit");
}

void Updater::show_complete_progress() {
    this->progress_bar->setValue(100);
}

