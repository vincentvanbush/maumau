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

    void gameListSignalHandle(struct game_list_msg);
    void joinOKSignalHandle(struct join_ok_msg);

    // information about existing games
    bool gameExists[50];
    int gameId[50];
    short playersCount[50];
    char playerNick[50][4][30];
    bool started[50];

    // information about current game
    short slotNumber;
    int playerToken;
    int gameToken;

//private slots:
public slots:
    void readMessage();







signals:
    void cannotJoinSignal();
    void joinOKSignal();

    void playerJoinedSignal();
    void starGameSignal();
    void nextTurnSignal();
    void invalidMoveSignal();
    void gameEndSignal();
    void playerLeftSignal();

    void gameListSignal();


private:
    QTcpSocket *tcpSocket;

    QString serverIPAddress;
    quint16 serverPort;

    int numberOfCardsInHand;


    std::vector<struct card> cardsList;

private slots:
    void socketError(QAbstractSocket::SocketError);
    void socketConnected();


};


#endif // TCPCLIENT

