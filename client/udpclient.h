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

//private slots:
public slots:
    void readMessage();

signals:
    void joinOKSignal(int);








private:
    QUdpSocket *socket;

    QString serverIPAddress;
    quint16 serverPort;

    int numberOfCardsInHand;


    std::vector<struct card> cardsList;



};


#endif // UDPCLIENT

