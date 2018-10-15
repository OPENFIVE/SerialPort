#-------------------------------------------------
#
# Project created by QtCreator 2018-01-25T20:37:43
#
#-------------------------------------------------
QT       += core gui
QT       += serialport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = SerialPort_v1.0
TEMPLATE = app
RC_ICONS = Port1.ico
# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        mywidget.cpp \
    myserial.cpp \
    iconbutton.cpp \
    qcustomplot.cpp

HEADERS  += mywidget.h \
    myserial.h \
    iconbutton.h \
    qcustomplot.h

FORMS    += mywidget.ui

CONFIG   += C++11

RESOURCES += \
    image.qrc

ICON = Port1.ico

DEFINES += QT_DLL QWT_DLL

