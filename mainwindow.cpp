#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QMessageBox>
#include <QThread>

/*
 * Конструктор класса
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connectWindow = new ConnectWindow(this);

    status = new QLabel(this);
    ui->statusbar->addWidget(status);

    this->setEnabled(false);
    this->setFixedSize(313, 412);
    showStatusMessage("");
    setWindowTitle("Smart Meter GUI");
}

/*
 * Деструктор класса
 */
MainWindow::~MainWindow()
{
    mainWindowPort->close();
    connectWindow->~ConnectWindow();

    delete ui;
}

//==================================================================

/*
 * Метод, вызывающий окно коннекта и ввода пароля
 */
void MainWindow::showConnectionWgt()
{
    connectWindow->show();
    connectWindow->setEnabled(true);
}

/*
 * Запуск работы основного окна. Вызывается после ввода пороля
 * Проходит проверку на открытие порта, если false -> возврат
 * к окну ввода пароля
 */
bool MainWindow::runMainWindow(QSerialPort *port, uint8_t passType)
{
    setPort(port);
    if(!openPort())
    {
        delete this->mainWindowPort;
        return false;
    }

    setSlots();
    setEnabled(true);
    setPassType(passType);
    this->show();
    return true;

}

//==================================================================

/*
 * Слот приема данных из порта
 */
void MainWindow::portReceive()
{
    symbolsFromPort.append(mainWindowPort->readAll());

    if(!symbolsFromPort.contains("\r\n")) return;

    int end = symbolsFromPort.lastIndexOf("\r\n");
    symbolsFromPort = symbolsFromPort.mid(0, end);
}

/*
 */
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (QMessageBox::Yes == QMessageBox::question(this, "Закрытие",
                          "Завершить работу?",
                          QMessageBox::Yes|QMessageBox::No))
    {
        if(this->mainWindowPort->isOpen())
        {
            QByteArray byteData;
            byteData.append("end_connection\r\n");
            this->mainWindowPort->write(byteData);
        }

        event->accept();
    }

}

/*
 * Слот нажатия кнопки "Получить данные"
 */
void MainWindow::on_getDataBtn_clicked()
{
    if(!mainWindowPort->isOpen()) return;
    mainWindowPort->write("get_data\r\n");

    waitForCmdTransmit(false);
    if(!getData_timer.isActive())
        getData_timer.start(3000);
}

/*
 * Слот кнопки "Получить время и дату"
 */
void MainWindow::on_getTimeDateBtn_clicked()
{
    if(!mainWindowPort->isOpen()) return;

    mainWindowPort->write("get_time\r\n");

    waitForCmdTransmit(false);
    if(!getTime_timer.isActive())
        getTime_timer.start(500);
}

/*
 * Слот кнопки ""
 */

void MainWindow::on_computerDateTimeBtn_clicked()
{
    ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());
}

/*
 *
 */
void MainWindow::on_setTimeBtn_clicked()
{
    if(!mainWindowPort->isOpen()) return;

    QTime time = ui->dateTimeEdit->time();

    uint32_t timeToMeter = 0;
    timeToMeter += time.hour() * 10000;
    timeToMeter += time.minute() * 100;
    timeToMeter += time.second();

    QString message = "set_time:" + QString::number(timeToMeter) + "\r\n";

    waitForCmdTransmit(true);
    mainWindowPort->write(message.toUtf8());
}

/*
 *
 */
void MainWindow::on_setDateBtn_clicked()
{
    if(!mainWindowPort->isOpen()) return;

    QDate date = ui->dateTimeEdit->date();

    uint32_t dateToMeter = 0;
    dateToMeter += date.day() * 10000;
    dateToMeter += date.month() * 100;
    dateToMeter += date.year() % 100;

    QString message = "set_date:" + QString::number(dateToMeter) + "\r\n";

    waitForCmdTransmit(true);
    mainWindowPort->write(message.toUtf8());
}

/*
 * Слот кнопки "Выйти". Происходит открытие окна ввода пароля
 */
void MainWindow::on_qxitBtn_clicked()
{
    mainWindowPort->close();
    this->setEnabled(false);
    this->connectWindow->setEnabled(true);
    //todo свою show()
    connectWindow->show();
}

/*
 *
 */
void MainWindow::on_resetMeterBtn_clicked()
{
    if(!mainWindowPort->isOpen()) return;

    QString message = "reset_meters\r\n";

    waitForCmdTransmit(true, 2000);
    mainWindowPort->write(message.toUtf8());
}

/*
 *
 */
void MainWindow::on_getSettingsBtn_clicked()
{

}


/*
 * Включение сезонного времени
 */
void MainWindow::on_onOffDaylightSavingBtn_clicked()
{
    if(!mainWindowPort->isOpen()) return;

    QString message = "set_daylight:1\r\n";

    mainWindowPort->write(message.toUtf8());
}

/*
 * Переход на зимнее время
 */
void MainWindow::on_changeToWinterBtn_clicked()
{
    if(!mainWindowPort->isOpen()) return;

    QString message = "change_daylight:1\r\n";

    mainWindowPort->write(message.toUtf8());
}

/*
 * Переход на летнее время
 */
void MainWindow::on_changeToSummerBtn_clicked()
{
    if(!mainWindowPort->isOpen()) return;

    QString message = "change_daylight:2\r\n";

    mainWindowPort->write(message.toUtf8());
}

/*
 *
 */
void MainWindow::on_calibrateBtn_clicked()
{
    if(!mainWindowPort->isOpen()) return;

    QString message = "calibrate\r\n";

    waitForCmdTransmit(false);
    mainWindowPort->write(message.toUtf8());
    calibration_timer.start(10000);
}

/*
 */
void MainWindow::on_setFirstBtn_clicked()
{
    if(!mainWindowPort->isOpen()) return;
    QString pass = ui->firstPassInput->text();
    if(pass.isEmpty())
    {
        showStatusMessage("Введите пароль");
        return;
    } else if(pass.size() <= 3)
    {
        showStatusMessage("Введите нормальный пароль)");
        return;
    }

    //обоработка пароля

    QString message = "set_first_pass:" + QString::number(123) + "\r\n";

    mainWindowPort->write(message.toUtf8());
    waitForCmdTransmit(true);
}

/*
 *
 */
void MainWindow::on_setSecondPassBtn_clicked()
{
    if(!mainWindowPort->isOpen()) return;
    QString pass = ui->secondPassInput->text();
    if(pass.isEmpty())
    {
        showStatusMessage("Введите пароль");
        return;
    } else if(pass.size() <= 3)
    {
        showStatusMessage("Введите нормальный пароль)");
        return;
    }

    //обоработка пароля

    QString message = "set_second_pass:" + QString::number(123) + "\r\n";

    mainWindowPort->write(message.toUtf8());
    waitForCmdTransmit(true);
}

/*
 *
 */
void MainWindow::on_getEventDataBtn_clicked()
{
    if(!mainWindowPort->isOpen()) return;

    QString message = "get_event_notes\r\n";

    mainWindowPort->write(message.toUtf8());

    getEvents_timer.start(1000);
}

//============================================================

/*
 * Слот таймера
 */
void MainWindow::getTime_timeoutSlot()
{
    getTime_timer.stop();

    QString message = "Время счетчика: ";
    QString timeString = "";
    QString dateString = "";

    for(int i = 0; i < 3; i++)
    {
        int temp = symbolsFromPort.at(i);
        if(temp < 10)
        {
            message += '0';
            timeString += '0';
        }
        QString tmp = QString::number(temp, 10);
        message += tmp;
        if (i != 2) message += ':';
        timeString += tmp;
    }

    message += " и дата: ";

    for(int i = 3; i < 6; i++)
    {
        int temp = symbolsFromPort.at(i);

        if(temp < 10)
        {
            message += '0';
            dateString += '0';
        }
        QString tmp = QString::number(temp, 10);
        message += tmp;
        if (i != 5) message += '.';
        if (i == 5) dateString += "20";
        dateString += tmp;
    }

    ui->dateTimeEdit->setTime(QTime::fromString(timeString, "hhmmss"));
    ui->dateTimeEdit->setDate(QDate::fromString(dateString, "ddMMyyyy"));
    symbolsFromPort.clear();

    stopWaitingCmdTransmit();
    showStatusMessage(message);
}

/*
 * Слот обработки операции получения данных
 */
void MainWindow::getData_timeoutSlot()
{
    getData_timer.stop();

    int countOfData = 18;
    double data[countOfData];

    for(int i = 0; i < countOfData; i++)
    {
        data[i] = convertFromHexToDouble(i);
        qDebug() << i << " " << data[i];
    }

    ui->dataOutput->setText(tr(dataField.toLatin1())
                            .arg(QString::number(data[0]))
                            .arg(QString::number(data[1]))
                            .arg(QString::number(data[2]))
                            .arg(QString::number(data[3]))
                            .arg(QString::number(data[4]))
                            .arg(QString::number(data[5]))
                            .arg(QString::number(data[6]))
                            .arg(QString::number(data[7]))
                            .arg(QString::number(data[8]))
                            .arg(QString::number(data[9]))
                            .arg(QString::number(data[10]))
                            .arg(QString::number(data[11]))
                            .arg(QString::number(data[12]))
                            .arg(QString::number(data[13]))
                            .arg(QString::number(data[14]))
                            .arg(QString::number(data[15]))
                            .arg(QString::number(data[16]))
                            .arg(QString::number(data[17])));
    stopWaitingCmdTransmit();
}

void MainWindow::getSettings_timeoutSlot()
{
    getSettings_timer.stop();
}

/*
 *
 */
void MainWindow::stopWaitingCmdTransmit_timeoutSlot()
{
    wait_timer.stop();
    this->setEnabled(true);
    showStatusMessage("Готово");
}

void MainWindow::caliration_timeoutSlot()
{
    calibration_timer.stop();
    if(symbolsFromPort.at(0) == 0)
    {
        showStatusMessage("Калибровка успешно завершена");
    } else {
        showStatusMessage(tr("Ошибка %1, смотрите AN366REV2").arg(symbolsFromPort.at(0)));
    }
}

void MainWindow::getEvent_timeoutSlot()
{
    getEvents_timer.stop();

    if(symbolsFromPort.isEmpty()) return;

    uint32_t seconds = 0, duration = 0;
    uint8_t info = 0, event = 0;

    int index = 0;
    int count = 1;
    int size_of_symbols = symbolsFromPort.size() - 1;

    while(index + 11 <= size_of_symbols)
    {
        seconds = 0;
        info = 0;
        event = 0;
        duration = 0;
        while(symbolsFromPort.at(index) != ':')
        {
            //парсим секунды с 2000 года
            for(int i = 3; i >= 0; i--)
            {
                if( i == 0)
                {
                    seconds = ( seconds + symbolsFromPort.at(i));
                } else {
                    seconds = ( seconds + symbolsFromPort.at(i)) << 8;
                }
                index++;
            }

            //поле информации
            info = symbolsFromPort.at(index++);

            //длительность
            for(int i = 3; i >= 0; i--)
            {
                if( i == 0)
                {
                    duration = (duration + symbolsFromPort.at(i));
                } else {
                    duration = (duration + symbolsFromPort.at(i)) << 8;
                }
                index++;
            }
            //номер события
            event = symbolsFromPort.at(index++);
        }
        index++;
        ui->eventOutput->append(tr("%1: %2 -> %3 -> %4 ->%5\n").arg(count).arg(event).arg(seconds).arg(info).arg(duration));
        count++;
    }

}

//==================================================================

/*
 * Вывод сообщения в StatusBar
 */
void MainWindow::showStatusMessage(const QString &message)
{
    QString statusMessage = "Status: " + message;
    status->setText(statusMessage);
}

/*
 * Настройка порта. Принимает port, пришедший из окна ввода пароля
 */
void MainWindow::setPort(QSerialPort *port)
{
    mainWindowPort = new QSerialPort(this);
    mainWindowPort->setPortName(port->portName());
    mainWindowPort->setBaudRate(port->baudRate());
    mainWindowPort->setDataBits(QSerialPort::Data8);
    mainWindowPort->setParity(QSerialPort::NoParity);
    mainWindowPort->setFlowControl(QSerialPort::NoFlowControl);
    mainWindowPort->setStopBits(QSerialPort::OneStop);
}

/*
 *
 */
void MainWindow::waitForCmdTransmit(bool withTimer, int timeout)
{
    showStatusMessage("Подождите...");
    this->setEnabled(false);
    if(withTimer)
        wait_timer.start(timeout);
}

/*
 *
 */
void MainWindow::stopWaitingCmdTransmit()
{
    this->setEnabled(true);
    showStatusMessage("Готово");
}

/*
 *
 */
void MainWindow::setPassType(uint8_t passType)
{
    _passType = passType;
}

/*
 * Открытие порта. При неудаче TODO
 */
bool MainWindow::openPort()
{
    if(!this->mainWindowPort->open(QIODevice::ReadWrite))
    {
        showStatusMessage(tr("Failed to open port").arg(mainWindowPort->portName()));
        return false;
    }

    return true;
}

/*
 * Установка слотов
 */
void MainWindow::setSlots()
{
    connect(mainWindowPort, SIGNAL(readyRead()), this, SLOT(portReceive()));
    connect(&getTime_timer, SIGNAL(timeout()), this, SLOT(getTime_timeoutSlot()));
    connect(&getData_timer, SIGNAL(timeout()), this, SLOT(getData_timeoutSlot()));
    connect(&getSettings_timer, SIGNAL(timeout()), this, SLOT(getSettings_timeoutSlot()));
    connect(&wait_timer, SIGNAL(timeout()), this, SLOT(stopWaitingCmdTransmit_timeoutSlot()));
    connect(&calibration_timer, SIGNAL(timeout()), this, SLOT(caliration_timeoutSlot()));
    connect(&getEvents_timer, SIGNAL(timeout()), this, SLOT(getEvent_timeoutSlot()));
}

double MainWindow::convertFromHexToDouble(int indexNumber)
{
    if(symbolsFromPort.isEmpty()) return 0.0;

    double output = 0.0;
    std::string hexString;
    int index = 8 * indexNumber + indexNumber + 7;

    while( symbolsFromPort.at(index) != ':' )
    {
        uint8_t temp = symbolsFromPort.at(index);

        if(temp < 10)
        {
            hexString += '0';
        }

        QString tmp = QString::number(temp, 16);
        hexString += tmp.toStdString();

        index--;
        if(index == -1) break;
    }

    try{
        *reinterpret_cast<unsigned long long*>(&output) = std::stoull(hexString, nullptr, 16);
    }
    catch(...){

        showStatusMessage("Error with double");
        return 0.0;
    }

    hexString.clear();

    return output;
}

