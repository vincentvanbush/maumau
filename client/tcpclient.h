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

    void gameListSignalHandle(struct game_list_msg);
    void joinOKSignalHandle(struct join_ok_msg);
    void startGameSignalHandle(struct start_game_msg);
    void playerJoinedSignalHandle(struct player_joined_msg);
    void gameEndSignalHandle(struct game_end_msg);
    void playerLeftSignalHandle(struct player_left_msg);
    void cannotJoinSignalHandle(struct cannot_join_msg);
    void cannotLeaveSignalHandle(struct cannot_leave_msg);
    void cannotReadySignalHandle(struct cannot_ready_msg);

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
    struct card firstCardInStack;
    short moveAtSlot;

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

