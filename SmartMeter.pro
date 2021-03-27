QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

VERSION = 1.0.0.1
QMAKE_TARGET_COMPANY = NoCompany
QMAKE_TARGET_PRODUCT = SmartMeter GUI Project
QMAKE_TARGET_DESCRIPTION = Description

RC_ICONS += icon.ico

CONFIG += c++11

SOURCES += \
    connectwindow.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    connectwindow.h \
    mainwindow.h

FORMS += \
    connectwindow.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

