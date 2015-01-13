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
    udpclient.cpp

HEADERS  += mainwindow.h \
    udpclient.h

FORMS    += mainwindow.ui
