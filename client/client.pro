#-------------------------------------------------
#
# Project created by QtCreator 2015-01-12T13:12:04
#
#-------------------------------------------------

QT       += core gui network quick declarative quickwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = client
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    tcpclient.cpp

HEADERS  += mainwindow.h \
    tcpclient.h

FORMS    += mainwindow.ui

CONFIG += c++11

DISTFILES += \
    interface.qml

unix:!macx: LIBS += -ljsoncpp

INCLUDEPATH += /usr/include/jsoncpp/
