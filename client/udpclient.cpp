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


    switch(messageType) {
    case CANNOT_JOIN:
        break;
    case JOIN_OK:
        break;
    case PLAYER_JOINED:
        break;
    case START_GAME:
        break;
    case NEXT_TURN:
        break;
    case INVALID_MOVE:
        break;
    case GAME_END:
        break;
    case PLAYER_LEFT:
        break;
    case GAME_LIST:
        this->gameListSignalHandle(gameMessage.message.game_list);
        this->gameListSignal();
        break;
    }

    if(messageType == JOIN_OK) {
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

void UdpClient::gameListSignalHandle(struct game_list_msg gameList)
{
    for(int i=0; i<50; i++) {
        gameExists[i] = gameList.game_exists[i];
        gameId[i] = gameList.game_id[i];
        playersCount[i] = gameList.players_count[i];
        for(int j=0; j<playersCount[i]; j++) {
            strcpy(playerNick[i][j], gameList.player_nick[i][j]);
        }
        started[i] = gameList.started[i];
    }
}

