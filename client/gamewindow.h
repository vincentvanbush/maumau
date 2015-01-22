#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QDialog>
#include <QLayout>
#include <QLabel>
#include <map>
#include "../messages.h"
#include "tcpclient.h"

namespace Ui {
class GameWindow;
}

class GameWindow : public QDialog
{
    Q_OBJECT

public:
    explicit GameWindow(TcpClient *tcpClient, Json::Value &join_ok_msg, QWidget *parent = 0);
    ~GameWindow();

private:
    Ui::GameWindow *ui;
    void addCardToLayout(int, int, QLayout*);
    QString getCardGfxFileName(int, int);
    TcpClient *tcpClient;
    std::vector<card>* readCards();
    short readRequest();
    void updateCards(int turn, bool covered = false);
    void updateTable();
    QString valueString(int);
    QString colorString(int);
    QLayout *getCardLayoutForSlot(int);
    QLabel *getLabelForSlot(int);
    std::vector<card> cardsInHand;
    std::vector<card> cardsInTable;

    int mySlot;
    std::map<int, QString> player_names;
    int currentTurn;
    std::map<int, int> cardCounts;

private slots:
    void on_moveButton_clicked();
    void on_readyButton_clicked();

    void onPickCardsMessageRecv(Json::Value &);
    void onStartGameMessageRecv(Json::Value &);
    void onPlayerJoinedMessageRecv(Json::Value &);
    void onMoveMessageRecv(Json::Value &);
    void onNextTurnMessageRecv(Json::Value &);
    void onInvalidMoveMessageRecv(Json::Value&);
};

#endif // GAMEWINDOW_H
