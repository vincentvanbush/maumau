#-------------------------------------------------
#
# Project created by QtCreator 2015-01-12T13:12:04
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = client
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    tcpclient.cpp \
    gamewindow.cpp \
    cardlabel.cpp

HEADERS  += mainwindow.h \
    tcpclient.h \
    gamewindow.h \
    cardlabel.h

FORMS    += mainwindow.ui \
    gamewindow.ui

CONFIG += c++11

DISTFILES += \
    interface.qml

unix:!macx: LIBS += -ljsoncpp

INCLUDEPATH += /usr/include/jsoncpp/

RESOURCES += \
    res.qrc

QMAKE_CXXFLAGS += -Wall
