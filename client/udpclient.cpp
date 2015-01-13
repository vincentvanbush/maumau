#include "udpclient.h"


UdpClient::UdpClient()
{
    this->socket = new QUdpSocket(this);
    this->serverIPAddress = "127.0.0.1";
    this->serverPort = 1234;


    socket->bind(QHostAddress::LocalHost, qint16(12312));
    connect(socket, SIGNAL(readyRead()), this, SLOT(readMessage()));

}
UdpClient::~UdpClient()
{
}
void UdpClient::readMessage()
{
    struct game_msg gameMessage;
    QHostAddress addr (serverIPAddress);

    socket->readDatagram((char *) &gameMessage, (qint64) sizeof(gameMessage), &addr, &serverPort);

    int messageType = gameMessage.msg_type;

    qDebug() << messageType;


    //
    if(messageType == JOIN_OK) { //
        joinOKSignal(messageType);
    }

}

void UdpClient::sendJoinGameMessage(std::string playerName)
{
    struct game_msg gameMessage;
    gameMessage.msg_type = 0;

    const char* playerNameChar = playerName.c_str();

    strcpy(gameMessage.message.join_game.player_name, playerNameChar);
    gameMessage.message.join_game.game_id = 3;

    this->socket->writeDatagram((const char*) &gameMessage, sizeof(gameMessage), QHostAddress(serverIPAddress), serverPort);
}


void UdpClient::sendRequestGamesMessage()
{
    struct game_msg msg;
    msg.msg_type = 13;

    this->socket->writeDatagram((const char*) &msg, sizeof(msg), QHostAddress(serverIPAddress), serverPort);
}
