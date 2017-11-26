/* SPDX-License-Identifier: GPL-3.0 */

#ifndef FEEDBACKDIALOG_H
#define FEEDBACKDIALOG_H

#include <cstdint>

#include <QDialog>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QLineEdit>
#include <QProgressBar>
#include <QThread>

#include "../network/developer_email.h"
#include "../network/smtp_client.h"

namespace Ui {
class FeedbackDialog;
}

class FeedbackDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FeedbackDialog(QWidget *parent = 0);
    ~FeedbackDialog();
    void system_information(QString&);

private slots:
    void on_close_btn_clicked();
    void on_deliver_feedback();
    void on_text_changed();
    void on_thread_finished();
    void on_runner_done();
    void on_runner_error(SMTPClient::SMTPClientState);
    void on_runner_step(SMTPClient::SMTPClientState);
    void on_runner_success();
private:
    Ui::FeedbackDialog *ui;
    QPlainTextEdit *text_box;
    QPushButton *submit_btn;
    QComboBox *report_box;
    QLineEdit *name_line;
    QProgressBar *progress_bar;
    QString _system_information;
    QThread *client_thread;
    DeveloperEMail *current_mail;
    SMTPClient::SMTPClientState last_step;
    enum { FBD_WAITING, FBD_SENDING, FBD_SUCCEEDED, FBD_FAILED } state;

    void initialize_controls();
    DeveloperEMail* prepare_mail();
    QThread *prepare_thread(DeveloperEMail&);
    void post_submission_action();
    void set_elements_enabled(bool);
};

class ClientRunnerThread : public QThread {
  Q_OBJECT
  public:
    ClientRunnerThread(DeveloperEMail&, QObject *parent = 0);
  signals:
    void step(SMTPClient::SMTPClientState);
    void error(SMTPClient::SMTPClientState);
    void success();
  protected:
    void run();
  private:
    DeveloperEMail& mail;
};

#endif // FEEDBACKDIALOG_H
