#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
//#include <QtDeclarative/qdeclarativeview.h>
#include <QVBoxLayout>

#include <QStandardItemModel>
#include <QHeaderView>

//#include <QtQuick/qquickview.h>


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
    void onReadyButtonClicked();
    void onLeaveGameButtonClicked();
    void onSendMoveButtonClicked();



    void onCannotJoinMessageRecv();
    void onCannotReadyMessageRecv();
    void onCannotLeaveMessageRecv();
    void onJoinOKMessageRecv();
    void onPlayerJoinedMessageRecv();
    void onStartGameMessageRecv();
    void onNextTurnMessageRecv();
    void onPickCardsMessageRecv();
    void onInvalidMoveMessageRecv();
    void onGameEndMessageRecv();
    void onPlayerLeftMessageRecv();
    void onGameListMessageRecv();
    void onMoveMessageRecv();




private:
    Ui::MainWindow *ui;
    std::string convertCardValue(int);
    short readRequest();
    std::vector<card> *readCards();

    QStandardItemModel model;

    QModelIndex modelIndex;

    QStandardItem *gamesIds[50];
    QStandardItem *playerNames[50][4];
    QStandardItem *gameStarted[50];

    QStringList horizontalHeader;

    void fillGamesTable();


};

#endif // MAINWINDOW_H
