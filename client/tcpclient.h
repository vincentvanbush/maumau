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
    void sendReadyMessage(int, int, int);
    void sendLeaveGameMessage(int, int, int, int);
    void sendMoveMessage(int, int, int, short, card*, short, short);

    void sendMessage(Json::Value&);
    Json::Value& recvMessage(int);

    // information about existing games

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

