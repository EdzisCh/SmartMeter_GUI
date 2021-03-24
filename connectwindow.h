#ifndef CONNECTWINDOW_H
#define CONNECTWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QTimer>
#include <QLabel>

const QString ACKNOWLEGE = "ack\r\n";

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

private slots:
    void on_connectBtn_clicked();
    void portReceive();
    void on_logInBtn_clicked();
    void timeoutHandle();
    void on_password_returnPressed();

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
