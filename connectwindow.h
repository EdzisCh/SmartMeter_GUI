#ifndef CONNECTWINDOW_H
#define CONNECTWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QTimer>
#include <QLabel>
#include <QDebug>

const QString ACKNOWLEGE_FIRST_PASS = "1ack\r\n";
const QString ACKNOWLEGE_SECOND_PASS = "2ack\r\n";
const QString PASS_INCORRECT = "!ack\r\n";

namespace Ui {
class ConnectWindow;
}

class ConnectWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ConnectWindow(QWidget *parent = nullptr);
    ~ConnectWindow();
    QSerialPort* getPort();
    void setPort(QSerialPort* port);

private slots:
    void on_connectBtn_clicked();
    void portReceive();
    void on_logInBtn_clicked();
    void timeoutHandle();
    void on_password_returnPressed();
    void closeEvent(QCloseEvent *event);

private:
    void showMessage(const QString &message);

    Ui::ConnectWindow *ui;
    QSerialPort *port = nullptr;
    QByteArray symbolsFromPort;
    QString dataFromPort;
    QTimer timer;
    QLabel *status;
};

#endif // CONNECTWINDOW_H
