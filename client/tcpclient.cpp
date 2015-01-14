#include "tcpclient.h"


TcpClient::TcpClient()
{
    this->serverIPAddress = "127.0.0.1";
    this->serverPort = 1234;

    this->tcpSocket = new QTcpSocket(this);
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));
    connect(tcpSocket, SIGNAL(connected()), this, SLOT(socketConnected()));
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readMessage()));
    tcpSocket->connectToHost(this->serverIPAddress, this->serverPort);

}
TcpClient::~TcpClient()
{
}
void TcpClient::readMessage()
{
    struct game_msg gameMessage;
    QHostAddress addr (serverIPAddress);

    tcpSocket->read((char *) &gameMessage, (qint64) sizeof(gameMessage));

    int messageType = gameMessage.msg_type;

    qDebug() << messageType;


    switch(messageType) {
    case CANNOT_JOIN:
        this->cannotJoinSignal();
        break;
    case JOIN_OK:
        this->joinOKSignalHandle(gameMessage.message.join_ok);
        this->joinOKSignal();
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

}

void TcpClient::sendJoinGameMessage(std::string playerName, int gameID)
{
    struct game_msg gameMessage;
    gameMessage.msg_type = 0;

    const char* playerNameChar = playerName.c_str();

    strcpy(gameMessage.message.join_game.player_name, playerNameChar);
    gameMessage.message.join_game.game_id = gameID;

    this->tcpSocket->write((const char*) &gameMessage, (qint64) sizeof(gameMessage));
}


void TcpClient::sendRequestGamesMessage()
{
    struct game_msg gameMessage;
    gameMessage.msg_type = 13;

    this->tcpSocket->write((const char*) &gameMessage, (qint64) sizeof(gameMessage));
}

void TcpClient::gameListSignalHandle(struct game_list_msg gameList)
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

void TcpClient::joinOKSignalHandle(struct join_ok_msg joinOK)
{
    this->slotNumber = joinOK.slot_number;
    this->playerToken = joinOK.player_token;
    this->gameToken = joinOK.game_token;
}

void TcpClient::socketError(QAbstractSocket::SocketError err)
{
    qDebug() << "Problem with socket";
    qDebug() << err;
}

void TcpClient::socketConnected()
{
    qDebug() << "Connection completed";
}
