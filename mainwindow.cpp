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
    mainWindowPort->write("get_data\r\n");

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
void MainWindow::on_dateTimeFromMeterBtn_clicked()
{
    if(!mainWindowPort->isOpen()) return;
    mainWindowPort->write("getDateTime\r\n");
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

    QString hexDouble;
    for(int i : symbolsFromPort)
    {
        if(i > 0 && i < 10) hexDouble.append('0');
        hexDouble.append(QString::number(i, 16));
        qDebug() << i;
    }
    qDebug() << "summary" << hexDouble;
    QString message = "value is : ";
    //...
    //ui->dataOutput->setText(tr("").arg(message).arg(tmp));
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
        //exit or msg
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

