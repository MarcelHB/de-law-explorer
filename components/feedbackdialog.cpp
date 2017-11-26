/* SPDX-License-Identifier: GPL-3.0 */

#include <QLayout>
#include <QMessageBox>

#include "feedbackdialog.h"
#include "ui_feedbackdialog.h"

FeedbackDialog::FeedbackDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FeedbackDialog),
    text_box(nullptr),
    submit_btn(nullptr),
    report_box(nullptr),
    name_line(nullptr),
    progress_bar(nullptr),
    client_thread(nullptr),
    state(FBD_WAITING)
{
    ui->setupUi(this);
    this->initialize_controls();
}

void FeedbackDialog::system_information(QString &str) {
    this->_system_information = QString(str);
}

void FeedbackDialog::initialize_controls() {
    this->setFixedSize(this->size());
    this->setWindowTitle("Feedback");

    this->report_box = this->findChild<QComboBox*>("comboBox");
    report_box->addItem("Feedback", QVariant::fromValue(DeveloperEMail::TYPE_FEEDBACK));
    report_box->addItem("Fehler", QVariant::fromValue(DeveloperEMail::TYPE_ERROR));

    this->text_box = this->findChild<QPlainTextEdit*>("plainTextEdit");
    connect(text_box, SIGNAL(textChanged()),
            this, SLOT(on_text_changed()));
    this->text_box->setFocus();

    this->submit_btn = this->findChild<QPushButton*>("pushButton_2");
    connect(this->submit_btn, SIGNAL(clicked()),
            this, SLOT(on_deliver_feedback()));

    QPushButton *cancel_btn = this->findChild<QPushButton*>("pushButton");
    connect(cancel_btn, SIGNAL(clicked()),
            this, SLOT(on_close_btn_clicked()));

    this->name_line = this->findChild<QLineEdit*>("lineEdit");
    this->progress_bar = this->findChild<QProgressBar*>("progressBar");
}

void FeedbackDialog::on_close_btn_clicked() {
    /*if(this->state == FBD_SENDING) {
        this->client_thread->terminate();
        this->clear_thread();
    }*/
    this->close();
}

void FeedbackDialog::on_deliver_feedback() {
    DeveloperEMail *mail = this->prepare_mail();
    QThread *thread = this->prepare_thread(*mail);

    this->set_elements_enabled(false);
    thread->start();
}

void FeedbackDialog::on_thread_finished() {
    delete this->client_thread;
    this->client_thread = nullptr;
    delete this->current_mail;
    this->current_mail = nullptr;

    this->post_submission_action();
}

void FeedbackDialog::on_text_changed() {
    QString text = this->text_box->toPlainText();
    this->submit_btn->setEnabled(text.size() >= 5);
}

DeveloperEMail* FeedbackDialog::prepare_mail() {
    QVariant type_var =
            this->report_box->itemData(this->report_box->currentIndex());
    DeveloperEMail::DeveloperMailType type =
            type_var.value<DeveloperEMail::DeveloperMailType>();

    QString message("USER: ");
    message.append(this->name_line->text());
    message.append("\r\n\r\n");
    message.append("MESSAGE: \n");
    message.append(this->text_box->toPlainText());

    this->current_mail = new DeveloperEMail(type, message, this->_system_information);
    return this->current_mail;
}

QThread* FeedbackDialog::prepare_thread(DeveloperEMail &mail) {
    this->client_thread = new ClientRunnerThread(mail, this);
    connect(this->client_thread, SIGNAL(step(SMTPClient::SMTPClientState)),
            this, SLOT(on_runner_step(SMTPClient::SMTPClientState)));
    connect(this->client_thread, SIGNAL(error(SMTPClient::SMTPClientState)),
            this, SLOT(on_runner_error(SMTPClient::SMTPClientState)));
    connect(this->client_thread, SIGNAL(success()),
            this, SLOT(on_runner_success()));
    connect(this->client_thread, SIGNAL(finished()),
            this, SLOT(on_thread_finished()));

    return this->client_thread;
}

void FeedbackDialog::on_runner_done() {
    this->post_submission_action();
}

void FeedbackDialog::on_runner_error(SMTPClient::SMTPClientState step) {
    this->state = FBD_FAILED;
    this->last_step = step;
}

void FeedbackDialog::on_runner_step(SMTPClient::SMTPClientState step) {
    int partial = 100/SMTPClient::HALT;
    this->progress_bar->setValue(partial * (step + 1));
}

void FeedbackDialog::on_runner_success() {
    this->state = FBD_SUCCEEDED;
}

void FeedbackDialog::set_elements_enabled(bool state) {
    this->text_box->setEnabled(state);
    this->submit_btn->setEnabled(state);
    this->name_line->setEnabled(state);
    this->report_box->setEnabled(state);
}

void FeedbackDialog::post_submission_action() {
    if(this->state == FBD_FAILED) {
        QString error_message("Feedback konnte nicht gesendet werden (Code ");
        error_message.append(QString::number(this->last_step));
        error_message.append(").");

        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText(error_message);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();

        this->set_elements_enabled(true);
        this->progress_bar->setValue(0);
    } else {
        this->progress_bar->setValue(100);

        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText("Nachricht gesendet. Vielen Dank!");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        this->close();
    }
}

FeedbackDialog::~FeedbackDialog()
{
    delete ui;
}

ClientRunnerThread::ClientRunnerThread(DeveloperEMail &_mail, QObject *parent) :
    QThread(parent),
    mail(_mail)
{}

void ClientRunnerThread::run() {
    SMTPClient client(this->mail);
    qRegisterMetaType<SMTPClient::SMTPClientState>("SMTPClient::SMTPClientState");

    connect(&client, SIGNAL(step(SMTPClient::SMTPClientState)),
            this, SIGNAL(step(SMTPClient::SMTPClientState)));
    connect(&client, SIGNAL(error(SMTPClient::SMTPClientState)),
            this, SIGNAL(error(SMTPClient::SMTPClientState)));
    connect(&client, SIGNAL(success()),
            this, SIGNAL(success()));
    connect(&client, SIGNAL(socket_closed()),
            this, SLOT(quit()));
    client.send();

    this->exec();
}
