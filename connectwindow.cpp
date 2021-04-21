#include "connectwindow.h"
#include "ui_connectwindow.h"
#include <QSerialPortInfo>
#include <QThread>
#include "mainwindow.h"
#include <QMessageBox>
#include <QCloseEvent>

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
    ui->password->setText("123");
    /**/
    ui->password->setEnabled(false);
    ui->logInBtn->setEnabled(false);
    ui->connectBtn->setFocus();
    setWindowTitle("Подключиться");
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
    {
        QByteArray byteData;
        byteData.append("end_connection\r\n");
        this->port->write(byteData);
        this->port->close();
    }

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

/*
 * Сеттер для порта
 */
void ConnectWindow::setPort(QSerialPort* port)
{
    this->port = port;
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
        showMessage(tr("Невозможно открыть %1").arg(this->ui->portNameComboBox->currentText()));
    } else {
        showMessage(tr("Подключенно к %1").arg(this->ui->portNameComboBox->currentText()));
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

    QString data = "connect:";

    if(ui->password->text().isEmpty())
    {
        showMessage("Введите пароль");
        return;
    }
    data += ui->password->text();//
    data += "\r\n";

    QByteArray byteData;
    byteData.append(data.toLocal8Bit());

    this->port->write(byteData);
    if(!timer.isActive())
        timer.start(1000);
}

void ConnectWindow::timeoutHandle()
{
    timer.stop();
    if(dataFromPort.compare(PASS_INCORRECT) == 0)
    {
        showMessage("Неправильный пароль");
        return;
    } else if(dataFromPort.compare(ACKNOWLEGE_FIRST_PASS) != 0 && dataFromPort.compare(ACKNOWLEGE_SECOND_PASS) != 0)
    {
        showMessage("Что-то пошло не так...");
        return;
    }

    showMessage("Вход выполнен");
    port->close();

    uint8_t passType = 0;
    if(dataFromPort.compare(ACKNOWLEGE_FIRST_PASS) == 0) passType = 1;
    if(dataFromPort.compare(ACKNOWLEGE_SECOND_PASS) == 0) passType = 2;

    if(!static_cast<MainWindow *>(this->parent())->runMainWindow(port, passType))
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

/*
 */
void ConnectWindow::closeEvent(QCloseEvent *event)
{
    static_cast<MainWindow *>(this->parent())->close();
//    //event->ignore();
//    if (QMessageBox::Yes == QMessageBox::question(this, "Закрытие",
//                          "Завершить работу?",
//                          QMessageBox::Yes|QMessageBox::No))
//    {
//        if(!this->port->isOpen())
//            this->port->open(QIODevice::ReadWrite);

//        QByteArray byteData;
//        byteData.append("end_connection\r\n");
//        this->port->write(byteData);

//        this->port->close();

//        event->accept();
//        static_cast<MainWindow *>(this->parent())->close();
//    }

}

//==================================================================

/*
 * Показать сообщение в StatusBar
 */
void ConnectWindow::showMessage(const QString &message)
{
    QString statusMessage = "Статус: " + message;
    status->setText(statusMessage);
}

