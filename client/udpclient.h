#ifndef UDPCLIENT
#define UDPCLIENT


#include <QtNetwork/QHostInfo>
#include <QtNetwork/QUdpSocket>
#include <QtNetwork/QHostAddress>

#include "../games.h"
#include "../messages.h"

#include <vector>
#include <string>



class UdpClient : public QObject {
    Q_OBJECT

public:
    UdpClient();
    ~UdpClient();
    void sendRequestGamesMessage();
    void sendJoinGameMessage(std::string);

    void gameListSignalHandle(struct game_list_msg);

    // information about existing games
    bool gameExists[50];
    int gameId[50];
    short playersCount[50];
    char playerNick[50][4][30];
    bool started[50];

//private slots:
public slots:
    void readMessage();







signals:
    void cannotJoinSignal();
    void joinOKSignal(int);
    void playerJoinedSignal();
    void starGameSignal();
    void nextTurnSignal();
    void invalidMoveSignal();
    void gameEndSignal();
    void playerLeftSignal();
    void gameListSignal();


private:
    QUdpSocket *socket;

    QString serverIPAddress;
    quint16 serverPort;

    int numberOfCardsInHand;


    std::vector<struct card> cardsList;



};


#endif // UDPCLIENT

