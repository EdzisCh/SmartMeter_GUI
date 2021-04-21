#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QTimer>
#include <QLabel>
#include "connectwindow.h"
#include "definitions.h"
#include <QCloseEvent>

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
    bool runMainWindow(QSerialPort *port, uint8_t passType);

private slots:
    void portReceive();
    void closeEvent(QCloseEvent *event);

    void on_getDataBtn_clicked();
    void on_getTimeDateBtn_clicked();
    void on_qxitBtn_clicked();
    void on_computerDateTimeBtn_clicked();
    void on_setTimeBtn_clicked();
    void on_setDateBtn_clicked();
    void on_resetMeterBtn_clicked();
    void on_getSettingsBtn_clicked();
    void on_calibrateBtn_clicked();
    void on_setFirstBtn_clicked();
    void on_setSecondPassBtn_clicked();
    void on_onOffDaylightSavingBtn_clicked();
    void on_changeToWinterBtn_clicked();
    void on_changeToSummerBtn_clicked();

    void getTime_timeoutSlot();
    void getData_timeoutSlot();
    void getSettings_timeoutSlot();
    void stopWaitingCmdTransmit_timeoutSlot();
    void caliration_timeoutSlot();
    void getEvent_timeoutSlot();

    void on_getEventDataBtn_clicked();

private:
    void showStatusMessage(const QString &message);
    void setPort(QSerialPort *port);
    void setPassType(uint8_t passType);
    bool openPort();
    void setSlots();
    double convertFromHexToDouble(int indexNumber);
    void convertFromHexToTime();
    void waitForCmdTransmit(bool withTimer, int timeout = 1000);
    void stopWaitingCmdTransmit();

    Ui::MainWindow *ui;
    QSerialPort *mainWindowPort;
    QByteArray symbolsFromPort;
    QString dataFromPort;
    QLabel *status;
    QTimer getTime_timer;
    QTimer getData_timer;
    QTimer getSettings_timer;
    QTimer calibration_timer;
    QTimer getEvents_timer;
    QTimer wait_timer;
    uint8_t _passType;

    ConnectWindow *connectWindow;
};
#endif // MAINWINDOW_H
