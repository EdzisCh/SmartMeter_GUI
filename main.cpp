#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow mainWindow;
    mainWindow.setStyleSheet("QTextBar { background-color: yellow }");
    mainWindow.show();
    mainWindow.showConnectionWgt();
    return a.exec();
}
