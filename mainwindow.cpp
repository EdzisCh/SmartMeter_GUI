#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>

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
    this->setFixedSize(308, 376);
    showStatusMessage("");
    setWindowTitle("Smart Meter GUI");

}

/*
 * Деструктор класса
 */
MainWindow::~MainWindow()
{
    if(mainWindowPort->isOpen())
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
bool MainWindow::runMainWindow(QSerialPort *port)
{
    setPort(port);
    if(!openPort())
    {
        delete this->mainWindowPort;
        return false;
    }

    setSlots();
    setEnabled(true);
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
 * Слот нажатия кнопки "Получить данные"
 */
void MainWindow::on_getDataBtn_clicked()
{
    if(!mainWindowPort->isOpen()) return;
    mainWindowPort->write("get_data\r\n");

    if(!getData_timer.isActive())
        getData_timer.start(2000);
}

/*
 * Слот кнопки "Получить время и дату"
 */
void MainWindow::on_getTimeDateBtn_clicked()
{
    if(!mainWindowPort->isOpen()) return;

    mainWindowPort->write("get_time\r\n");

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

    mainWindowPort->write(message.toUtf8());
}

/*
 *
 */
void MainWindow::on_setDateTimeBtn_clicked()
{
    on_setTimeBtn_clicked();
    on_setDateBtn_clicked();
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
 * Слот таймера
 */
void MainWindow::getTime_timeoutSlot()
{
    getTime_timer.stop();

    QString message = "Current time: ";
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

    message += " and date: ";

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

    showStatusMessage(message);
}

/*
 * Слот обработки операции получения данных
 */
void MainWindow::getData_timeoutSlot()
{
    getData_timer.stop();

    int countOfData = 6;
    double data[countOfData];

    for(int i = 0; i < countOfData; i++)
    {
        data[i] = convertFromHexToDouble(i);
    }

    ui->dataOutput->setText(tr("Pavg = %1 Вт\nQavg = %2 ВА\nF = %3 Гц\nI = %4 A\nU = %5 B\nCos = %6")
                            .arg(QString::number(data[0]))
                            .arg(QString::number(data[1]))
                            .arg(QString::number(data[2]))
                            .arg(QString::number(data[3]))
                            .arg(QString::number(data[4]))
                            .arg(QString::number(data[5])));
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
