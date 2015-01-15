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
    void onLeaveGameButtonClicked();



    void onCannotJoinMessageRecv();
    void onCannotReadyMessageRecv();
    void onCannotLeaveMessageRecv();
    void onJoinOKMessageRecv();
    void onPlayerJoinedMessageRecv();
    void onStartGameMessageRecv();
    void onNextTurnMessageRecv();
    void onInvalidMoveMessageRecv();
    void onGameEndMessageRecv();
    void onPlayerLeftMessageRecv();
    void onGameListMessageRecv();




private:
    Ui::MainWindow *ui;

};

#endif // MAINWINDOW_H
