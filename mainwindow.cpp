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
    this->setFixedSize(311, 323);
    showStatusMessage("");
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
    qDebug() << "[]" << symbolsFromPort;
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
    //mainWindowPort->write("get_data\r\n");

    /*   */
    symbolsFromPort.clear();
    symbolsFromPort.append(0x40);
    symbolsFromPort.append(0x14);
    symbolsFromPort.append(0x98);
    symbolsFromPort.append(0x5f);
    symbolsFromPort.append(0x06);
    symbolsFromPort.append(0xf6);
    symbolsFromPort.append(0x94);
    symbolsFromPort.append(0x46);
    symbolsFromPort.append(':');
    symbolsFromPort.append(0x40);
    symbolsFromPort.append(0x14);
    symbolsFromPort.append(0x98);
    symbolsFromPort.append(0x5f);
    symbolsFromPort.append(0x06);
    symbolsFromPort.append(0xf6);
    symbolsFromPort.append(0x94);
    symbolsFromPort.append(0x46);
    symbolsFromPort.append(':');
    symbolsFromPort.append(0x40);
    symbolsFromPort.append(0x49);
    symbolsFromPort.append('0');
    symbolsFromPort.append('0');
    symbolsFromPort.append(0xfb);
    symbolsFromPort.append(0xa8);
    symbolsFromPort.append(0x82);
    symbolsFromPort.append(0x6b);
    symbolsFromPort.append(':');
    /*   */

    if(!getData_timer.isActive())
        getData_timer.start(500);
}

/*
 * Слот кнопки "Получить время"
 */
void MainWindow::on_getTimeBtn_clicked()
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
    qDebug() << message.toUtf8();
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
    qDebug() << message.toUtf8();
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

    QString message = "Current time in meter is ";
    QString timeString = "";

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
    ui->dateTimeEdit->setTime(QTime::fromString(timeString, "hhmmss"));
    symbolsFromPort.clear();

    ui->dataOutput->setText(message);
}

/*
 * Слот обработки операции получения данных
 */
void MainWindow::getData_timeoutSlot()
{
    getData_timer.stop();

    int countOfData = 3;
    double data[countOfData];

    for(int i = 0; i < countOfData; i++)
    {
        data[i] = convertFromHexToDouble(i);
    }

    ui->dataOutput->setText(tr("Pavg = %1 Вт\nQavg = %2 ВА\nF = %3 Гц")
                            .arg(QString::number(data[0]))
                            .arg(QString::number(data[1]))
                            .arg(QString::number(data[2])));

    symbolsFromPort.clear();
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
    int index = 8 * indexNumber + indexNumber;

    while( symbolsFromPort.at(index) != ':' )
    {
        uint8_t temp = symbolsFromPort.at(index);

        if(temp < 10)
        {
            hexString += '0';
        }

        QString tmp = QString::number(temp, 16);
        hexString += tmp.toStdString();

        index++;
    }

    index++;

    try{
        *reinterpret_cast<unsigned long long*>(&output) = std::stoull(hexString, nullptr, 16);
    }
    catch(...){
        qDebug() << "Error with double";
        showStatusMessage("Error with double");
        return 0.0;
    }

    hexString.clear();

    return output;
}
