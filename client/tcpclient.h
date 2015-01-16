#ifndef TCPCLIENT
#define TCPCLIENT


#include <QtNetwork/QHostInfo>
#include <QtNetwork/QUdpSocket>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QHostAddress>

#include "../games.h"
#include "../messages.h"

#include <vector>
#include <string>



class TcpClient : public QObject {
    Q_OBJECT

public:
    TcpClient();
    ~TcpClient();
    void sendRequestGamesMessage();
    void sendJoinGameMessage(std::string, int);
    void sendReadyMessage();
    void sendLeaveGameMessage();
    void sendMoveMessage(short, card*, short, short);

    void updateHandAfterMove(short, card*);


    void cannotJoinSignalHandle(struct cannot_join_msg);
    void cannotReadySignalHandle(struct cannot_ready_msg);
    void cannotLeaveSignalHandle(struct cannot_leave_msg);
    void joinOKSignalHandle(struct join_ok_msg);
    void playerJoinedSignalHandle(struct player_joined_msg);
    void startGameSignalHandle(struct start_game_msg);
    void nextTurnSignalHandle(struct next_turn_msg);
    void pickCardsSignalHandle(struct pick_cards_msg);
    //invalid move
    void gameEndSignalHandle(struct game_end_msg);
    void playerLeftSignalHandle(struct player_left_msg);
    void gameListSignalHandle(struct game_list_msg);
    void moveSignalHandle(struct move_msg);

    // information about existing games
    bool gameExists[50];
    int gameId[50];
    short playersCount[50];
    char playerNick[50][4][30];
    bool started[50];

    // information about player and current game
    short slotNumber;
    int playerToken;
    int gameToken;


    int gameIdentifier;
    char playerName[30];

    int slotOfLastJoinedPlayer;
    char nameOfLastJoinedPlayer[30];

    int slotOfLastLeftPlayer;
    char nameOfLastLeftPlayer[30];

    std::map<int, std::string*> playersAtSlots;


    // information about current hand
    int numberOfCardsInHand;
    std::vector<struct card> cardsInHand;
    std::vector<struct card> cardsInStack;
    struct card firstCardInStack;
    short moveAtSlot;

    // information about move
    short playedCardsCount;
    struct card playedCards[4];

    short colorRequest;
    short valueRequest;

    short turn;
    short turns_for_next;
    short cards_for_next;

    short slot;
    short count;
    struct card cards[30];


//private slots:
public slots:
    void readMessage();


signals:
    void cannotJoinSignal();
    void cannotReadySignal();
    void cannotLeaveSignal();
    void joinOKSignal();
    void playerJoinedSignal();
    void starGameSignal();
    void nextTurnSignal();
    void pickCardsSignal();
    void invalidMoveSignal();
    void gameEndSignal();
    void playerLeftSignal();
    void gameListSignal();
    void moveSignal();











private:
    QTcpSocket *tcpSocket;
    QString serverIPAddress;
    quint16 serverPort;


private slots:
    void socketError(QAbstractSocket::SocketError);
    void socketConnected();


};


#endif // TCPCLIENT

