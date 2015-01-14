#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "tcpclient.h"
#include "../messages.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    TcpClient *tcpClient;
    
//private slots:
public slots:
    void onRequestGamesButtonClicked();
    void onJoinGameButtonClicked();
    void onReadyButtonClicked();

    void onJoinOKMessageRecv();
    void onGameListMessageRecv();
    void onCannotJoinMessageRecv();
    void onInvalidMoveMessageRecv();
    void onStartGameMessageRecv();




private:
    Ui::MainWindow *ui;

};

#endif // MAINWINDOW_H
