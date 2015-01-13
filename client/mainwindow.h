#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "udpclient.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    UdpClient *udpClient;
    
//private slots:
public slots:
    void onRequestGamesButtonClicked();
    void onJoinGameButtonClicked();

    void onJoinOKMessageRecv();
    void onGameListMessageRecv();
    void onCannotJoinMessageRecv();




private:
    Ui::MainWindow *ui;

};

#endif // MAINWINDOW_H
