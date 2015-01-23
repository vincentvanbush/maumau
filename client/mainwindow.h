#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItem>
#include <QStandardItemModel>

#include "gamewindow.h"
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
    void onNewGameButtonClicked();

    void onCannotJoinMessageRecv(Json::Value &);
    void onJoinOKMessageRecv(Json::Value &);
    void onGameListMessageRecv(Json::Value &);

private slots:
    void onSocketErrorSignal(QAbstractSocket::SocketError err);

private:
    Ui::MainWindow *ui;
    std::string convertCardValue(int);
    short readRequest();
    std::vector<card> *readCards();


    QStandardItem *gamesIds[50];
    QStandardItem *playerNames[50][4];
    QStandardItem *gameStarted[50];
    QStandardItemModel model;

    QStringList horizontalHeader;

    bool gameExists[50];
    int gameId[50];
    short playersCount[50];
    char playerNick[50][4][30];
    bool started[50];
    std::map<int, std::string*> playersAtSlots;

};

#endif // MAINWINDOW_H
