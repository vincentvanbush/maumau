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
    TcpClient(QString ip);
    ~TcpClient();
    void sendRequestGamesMessage();
    void sendJoinGameMessage(std::string, int);
    void sendReadyMessage();
    void sendLeaveGameMessage();
    void sendMoveMessage(short, card*, short, short);

    void updateHandAfterMove(short, card*);

    void sendMessage(Json::Value&);
    Json::Value& recvMessage(int);

    void cannotJoinSignalHandle(Json::Value&);
    void cannotReadySignalHandle(Json::Value&);
    void cannotLeaveSignalHandle(Json::Value&);
    void joinOKSignalHandle(Json::Value&);
    void playerJoinedSignalHandle(Json::Value&);
    void startGameSignalHandle(Json::Value&);
    void nextTurnSignalHandle(Json::Value&);
    void pickCardsSignalHandle(Json::Value&);
    //invalid move
    void gameEndSignalHandle(Json::Value&);
    void playerLeftSignalHandle(Json::Value&);
    void gameListSignalHandle(Json::Value&);
    void moveSignalHandle(Json::Value&);

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



//private slots:
public slots:
    void readMessage();


signals:
    void cannotJoinSignal(Json::Value &msg);
    void cannotReadySignal(Json::Value &msg);
    void cannotLeaveSignal(Json::Value &msg);
    void joinOKSignal(Json::Value &msg);
    void playerJoinedSignal(Json::Value &msg);
    void starGameSignal(Json::Value &msg);
    void nextTurnSignal(Json::Value &msg);
    void pickCardsSignal(Json::Value &msg);
    void invalidMoveSignal(Json::Value &msg);
    void gameEndSignal(Json::Value &msg);
    void playerLeftSignal(Json::Value &msg);
    void gameListSignal(Json::Value &msg);
    void moveSignal(Json::Value &msg);
    void socketErrorSignal(QAbstractSocket::SocketError);

private:
    QTcpSocket *tcpSocket;
    QString serverIPAddress;
    quint16 serverPort;
    bool socketGone;

private slots:
    void socketError(QAbstractSocket::SocketError);
    void socketConnected();
    void socketDestroyed();

};


#endif // TCPCLIENT

