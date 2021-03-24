#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QTimer>
#include <QLabel>
#include "connectwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void showConnectionWgt();
    bool runMainWindow(QSerialPort *port);

private slots:
    void portReceive();
    void on_getDataBtn_clicked();
    void on_getTimeBtn_clicked();
    void on_dateTimeFromMeterBtn_clicked();
    void getTime_timeoutSlot();
    void getData_timeoutSlot();
    void on_qxitBtn_clicked();

private:
    void showStatusMessage(const QString &message);
    void setPort(QSerialPort *port);
    bool openPort();
    void setSlots();

    Ui::MainWindow *ui;
    QSerialPort *mainWindowPort;
    QByteArray symbolsFromPort;
    QString dataFromPort;
    QLabel *status;
    QTimer getTime_timer;
    QTimer getData_timer;

    ConnectWindow *connectWindow;
};
#endif // MAINWINDOW_H
