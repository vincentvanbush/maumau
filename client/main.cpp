#include "mainwindow.h"
#include "gamewindow.h"
#include <QApplication>
#include "tcpclient.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    w.show();
    
    return a.exec();
}
