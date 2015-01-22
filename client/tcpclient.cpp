#include "tcpclient.h"
#include <json/json.h>

TcpClient::TcpClient(QString ip)
{
    this->serverIPAddress = ip;
    this->serverPort = 1234;

    for(int i=0; i<4; i++) {
        this->playersAtSlots[i] = nullptr;
    }

    for(int i=0; i<50; i++) {
        this->gameExists[i] = false;
        this->started[i] = false;
    }


    this->tcpSocket = new QTcpSocket(this);
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));
    connect(tcpSocket, SIGNAL(connected()), this, SLOT(socketConnected()));
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readMessage()));
    tcpSocket->connectToHost(this->serverIPAddress, this->serverPort);

}
TcpClient::~TcpClient()
{
}

void TcpClient::sendMessage(Json::Value &msg) {
    Json::FastWriter writer;
    std::string msg_str = writer.write(msg);
    const char* msg_c = msg_str.c_str();
    int c_size = strlen(msg_c);
    qDebug() << "Sending " << c_size << " bytes...";

    this->tcpSocket->write((const char*)&c_size, (qint64) sizeof(int));

    int sentBytes = 0;
    while (sentBytes < c_size) {
        int x = this->tcpSocket->write(msg_c + sentBytes, (qint64) c_size - sentBytes);
        if (x > 0) qDebug() << "Sent " << x << " bytes";
        sentBytes += x;
    }
    qDebug("Total: %d bytes\n", sentBytes);

    qDebug("Sent message content: %s", msg_c);
    // this->tcpSocket->write(msg_c, (qint64) c_size);
}

Json::Value& TcpClient::recvMessage(int len) {
    qDebug() << "Receiving " << len << " bytes...";
    char *msg_buf = new char[len];
    int bytesRead = 0;
    int x;
    while (bytesRead < len) {
        x = tcpSocket->read(msg_buf + bytesRead, (qint64) len - bytesRead);
        if (x > 0) qDebug() << "Read " << x << " bytes";
        bytesRead += x;
    }
    qDebug("Total: %d bytes\n", bytesRead);
    std::string msg_str(msg_buf);
    Json::Reader reader;
    Json::Value *ret = new Json::Value();
    if (!reader.parse(msg_str, *ret)) {
        qDebug("Error parsing: %s", msg_str.c_str());
    }
    qDebug("Received message content: %s", msg_str.c_str());
    return *ret;
}


void TcpClient::readMessage()
{
    QHostAddress addr (serverIPAddress);

    do { // do while there is something to read, as after one message there may be another
        int msg_len;
        tcpSocket->read((char*)&msg_len, (qint64) sizeof(int));

        Json::Value gameMessage = recvMessage(msg_len);
        int messageType = gameMessage["msg_type"].asInt();

        qDebug() << "Message type: " + QString::number(messageType);

        switch(messageType) {
        case CANNOT_JOIN:
            emit cannotJoinSignal(gameMessage);
            break;
        case CANNOT_READY:
            emit cannotReadySignal(gameMessage);
            break;
        case CANNOT_LEAVE:
            emit cannotLeaveSignal(gameMessage);
            break;
        case JOIN_OK:
            this->joinOKSignalHandle(gameMessage);
            emit joinOKSignal(gameMessage);
            break;
        case PLAYER_JOINED:
            emit playerJoinedSignal(gameMessage);
            break;
        case START_GAME:
            emit starGameSignal(gameMessage);
            break;
        case NEXT_TURN:
            emit nextTurnSignal(gameMessage);
            break;
        case PICK_CARDS:
            emit pickCardsSignal(gameMessage);
            break;
        case INVALID_MOVE:
            emit invalidMoveSignal(gameMessage);
            break;
        case GAME_END:
            emit gameEndSignal(gameMessage);
            break;
        case PLAYER_LEFT:
            emit playerLeftSignal(gameMessage);
            break;
        case GAME_LIST:
            this->gameListSignalHandle(gameMessage);
            emit gameListSignal(gameMessage);
            break;
        case MOVE:
            emit moveSignal(gameMessage);
            break;
        }
    } while (tcpSocket->bytesAvailable());

}

// methods sending communicates to server
void TcpClient::sendJoinGameMessage(std::string playerName, int gameID)
{
    Json::Value join_game_msg;
    join_game_msg["msg_type"] = JOIN_GAME;
    join_game_msg["player_name"] = playerName.c_str();
    join_game_msg["game_id"] = gameID;

    sendMessage(join_game_msg);
}

void TcpClient::sendRequestGamesMessage()
{
    Json::Value gameMessage;
    gameMessage["msg_type"] = REQUEST_GAME_LIST;
    sendMessage(gameMessage);
}

void TcpClient::sendReadyMessage()
{
    Json::Value gameMessage;
    gameMessage["msg_type"] = READY;
    gameMessage["player_token"] = this->playerToken;
    gameMessage["game_token"] = this->gameToken;
    gameMessage["game_id"] = this->gameIdentifier;
    sendMessage(gameMessage);
}

void TcpClient::sendLeaveGameMessage()
{
    Json::Value gameMessage;
    gameMessage["msg_type"] = 2;
    gameMessage["player_token"] = this->playerToken;
    gameMessage["game_token"] = this->gameToken;
    gameMessage["game_id"] = this->gameIdentifier;
    gameMessage["slot"] = this->slotNumber;



    // updating info about this game
    this->playersCount[this->gameIdentifier]--;
    for(int i=0; i<4; i++) {
        this->playerNick[this->gameIdentifier][i][0] = '\0';
    }
    this->started[this->gameIdentifier] = false;

    sendMessage(gameMessage);
}

void TcpClient::sendMoveMessage(short playedCardsCount, card* playedCards, short colorRequest, short valueRequest)
{
    Json::Value gameMessage;

    gameMessage["played_cards_count"] = playedCardsCount;
    for(int i=0; i<playedCardsCount; i++) {
        gameMessage["played_cards"][i]["color"] = playedCards[i].color;
        gameMessage["played_cards"][i]["value"] = playedCards[i].value;
    }
    gameMessage["color_request"] = colorRequest;
    gameMessage["value_request"] = valueRequest;

    gameMessage["msg_type"] = 11;
    gameMessage["player_token"] = this->playerToken;
    gameMessage["game_token"] = this->gameToken;
    gameMessage["game_id"] = this->gameIdentifier;

    sendMessage(gameMessage);
}


// methods handling communicates from server
void TcpClient::gameListSignalHandle(Json::Value& msg)
{
    Json::FastWriter writer;
    std::string txt = writer.write(msg);
    qDebug() << QString::fromStdString(txt);
    Json::Value &games = msg["games"];
    std::string gam = writer.write(games);
    qDebug() << QString::fromStdString(gam);
    if(!games.isNull())
        for(unsigned i=0; i<games.size(); i++) {
            gameExists[i] = !games[i].isNull();
            gameId[i] = games[i]["id"].asInt();
            playersCount[i] = games[i]["players_count"].asInt();
            for(int j=0; j<playersCount[i]; j++) {
                strcpy(playerNick[i][j], games[i]["player_nicks"][j].asCString());
            }
            started[i] = games[i]["started"].asBool();
        }
}

void TcpClient::joinOKSignalHandle(Json::Value& joinOK)
{
    this->slotNumber = joinOK["slot_number"].asInt();
    this->playerToken = joinOK["player_token"].asInt();
    this->gameToken = joinOK["game_token"].asInt();
    this->gameIdentifier = joinOK["game_id"].asInt();

}


// methods establishing connection with server
void TcpClient::socketError(QAbstractSocket::SocketError err)
{
    qDebug() << "Problem with socket";
    qDebug() << err;
}

void TcpClient::socketConnected()
{
    qDebug() << "Connection completed";
}


