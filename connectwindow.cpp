#include "connectwindow.h"
#include "ui_connectwindow.h"
#include <QSerialPortInfo>
#include <QThread>
#include "mainwindow.h"

/*
 * Конструктор класса ввода пароля
*/
ConnectWindow::ConnectWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ConnectWindow)
{
    ui->setupUi(this);
    this->setParent(parent);

    status = new QLabel();
    ui->statusbar->addWidget(status);

    port = new QSerialPort(this);
    QList<QSerialPortInfo> portInfos =  QSerialPortInfo::availablePorts();

    if(!portInfos.empty())
    {
        for(const QSerialPortInfo &info : portInfos)
        {
            this->ui->portNameComboBox->addItem(info.portName());
        }

        this->ui->baudRateComboBox->addItem("600");
        this->ui->baudRateComboBox->addItem("2400");
        this->ui->baudRateComboBox->addItem("9600");
        this->ui->baudRateComboBox->addItem("115200");
    }
    /**/
    ui->password->setText("connect");
    /**/
    ui->password->setEnabled(false);
    ui->logInBtn->setEnabled(false);
    ui->connectBtn->setFocus();
    connect(port, SIGNAL(readyRead()), this, SLOT(portReceive()));
    connect(&timer, SIGNAL(timeout()), this, SLOT(timeoutHandle()));
    showMessage("");
    this->setFixedSize(284, 179);
}

/*
 * Деструктор
 */
ConnectWindow::~ConnectWindow()
{
    if(this->port->isOpen())
        this->port->close();
    delete ui;
}

//==================================================================

/*
 * Геттер для COM порта.
 */
QSerialPort* ConnectWindow::getPort()
{
    return this->port;
}

//==================================================================

/*
 * Слот для приема сообщений от устройства
 */
void ConnectWindow::portReceive()
{
    symbolsFromPort.append(port->readAll());
    if(!symbolsFromPort .contains("\r\n")) return;

    //received data handle, need to make anorher method
    int end = symbolsFromPort .lastIndexOf("\r\n") + 2;
    dataFromPort = symbolsFromPort .mid(0, end);
    symbolsFromPort .clear();
}

/*
 * Слот соединения с портом. Выбираются настройки и открывается порт
 */
void ConnectWindow::on_connectBtn_clicked()
{
    if(port->portName() != ui->portNameComboBox->currentText())
    {
        port->close();
    }

    QString name = ui->portNameComboBox->currentText();
    port->setPortName(name);
    port->setBaudRate(ui->baudRateComboBox->currentText().toInt());
    port->setDataBits(QSerialPort::Data8);
    port->setParity(QSerialPort::NoParity);
    port->setStopBits(QSerialPort::OneStop);
    port->setFlowControl(QSerialPort::NoFlowControl);

    if(!this->port->open(QIODevice::ReadWrite))
    {
        showMessage(tr("Can`t open %1").arg(this->ui->portNameComboBox->currentText()));
    } else {
        showMessage(tr("Connected to %1").arg(this->ui->portNameComboBox->currentText()));
        ui->connectBtn->setEnabled(false);
        ui->logInBtn->setEnabled(true);
        ui->password->setEnabled(true);
        ui->password->setFocus();
    }
}

/*
 * Слот для входа в систему
 * ТУДУ: проверка по поролю
 */
void ConnectWindow::on_logInBtn_clicked()
{
    if(!port->isOpen()) return;

    QString data(ui->password->text());

    if(data.isEmpty())
    {
        showMessage("No input data");
        return;
    }
    data += "\r\n";
    QByteArray byteData;
    byteData.append(data.toLocal8Bit());
    this->port->write(byteData);
    if(!timer.isActive())
        timer.start(1000);
}

void ConnectWindow::timeoutHandle()
{
    if(dataFromPort.compare(ACKNOWLEGE) != 0)
    {
        showMessage("Password incorrect");
        timer.stop();
        return;
    }

    showMessage("Logged in");
    timer.stop();
    port->close();
    if(!static_cast<MainWindow *>(this->parent())->runMainWindow(port))
    {
        port->open(QIODevice::ReadWrite);
        showMessage("Error");
        return;
    }
    ui->connectBtn->setEnabled(true);
    ui->logInBtn->setEnabled(false);
    ui->password->setEnabled(false);
    this->hide();
}

/*
 * Слот нажатия Enter на поле ввода
 */
void ConnectWindow::on_password_returnPressed()
{
    on_logInBtn_clicked();
}

//==================================================================

/*
 * Показать сообщение в StatusBar
 */
void ConnectWindow::showMessage(const QString &message)
{
    QString statusMessage = "Status: " + message;
    status->setText(statusMessage);
}

