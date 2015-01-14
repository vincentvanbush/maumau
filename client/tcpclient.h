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

    void gameListSignalHandle(struct game_list_msg);
    void joinOKSignalHandle(struct join_ok_msg);
    void startGameSignalHandle(struct start_game_msg);

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
    void joinOKSignal();
    void gameListSignal();
    void invalidMoveSignal();

    void playerJoinedSignal();
    void starGameSignal();
    void nextTurnSignal();
    void gameEndSignal();
    void playerLeftSignal();




private:
    QTcpSocket *tcpSocket;
    QString serverIPAddress;
    quint16 serverPort;


private slots:
    void socketError(QAbstractSocket::SocketError);
    void socketConnected();


};


#endif // TCPCLIENT

