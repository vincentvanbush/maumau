#include "tcpclient.h"
#include <json/json.h>
#include <QMessageBox>

TcpClient::TcpClient(QString ip)
{
    this->serverIPAddress = ip;
    this->serverPort = 1234;
    this->socketGone = false;

    this->tcpSocket = new QTcpSocket(this);
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));
    connect(tcpSocket, SIGNAL(connected()), this, SLOT(socketConnected()));
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readMessage()));
    connect(tcpSocket, SIGNAL(destroyed()), this, SLOT(socketDestroyed()));
    tcpSocket->connectToHost(this->serverIPAddress, this->serverPort);

}
TcpClient::~TcpClient()
{
}

void TcpClient::sendMessage(Json::Value &msg) {
    if (socketGone) return;

    Json::FastWriter writer;
    std::string msg_str = writer.write(msg);
    const char* msg_c = msg_str.c_str();
    int c_size = strlen(msg_c);
    qDebug() << "Sending " << c_size << " bytes...";

    int code = this->tcpSocket->write((const char*)&c_size, (qint64) sizeof(int));
    if (code < 0) return;

    int sentBytes = 0;
    while (sentBytes < c_size) {
        int x = this->tcpSocket->write(msg_c + sentBytes, (qint64) c_size - sentBytes);
        if (x > 0) qDebug() << "Sent " << x << " bytes";
        sentBytes += x;
    }
    qDebug("Total: %d bytes\n", sentBytes);

    qDebug("Sent message content: %s", msg_c);
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

void TcpClient::sendReadyMessage(int playerToken, int gameToken, int gameId)
{
    Json::Value gameMessage;
    gameMessage["msg_type"] = READY;
    gameMessage["player_token"] = playerToken;
    gameMessage["game_token"] = gameToken;
    gameMessage["game_id"] = gameId;
    sendMessage(gameMessage);
}

void TcpClient::sendLeaveGameMessage(int playerToken, int gameToken, int gameId, int slotNumber)
{
    Json::Value gameMessage;
    gameMessage["msg_type"] = LEAVE_GAME;
    gameMessage["player_token"] = playerToken;
    gameMessage["game_token"] = gameToken;
    gameMessage["game_id"] = gameId;
    gameMessage["slot"] = slotNumber;
    sendMessage(gameMessage);
}

void TcpClient::sendMoveMessage(int playerToken, int gameToken, int gameId, short playedCardsCount, card* playedCards, short colorRequest, short valueRequest)
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
    gameMessage["player_token"] = playerToken;
    gameMessage["game_token"] = gameToken;
    gameMessage["game_id"] = gameId;

    sendMessage(gameMessage);
}

// methods establishing connection with server
void TcpClient::socketError(QAbstractSocket::SocketError err)
{
    qDebug() << "Problem with socket";
    qDebug() << err;
    tcpSocket->deleteLater();
    emit socketErrorSignal(err);
}

void TcpClient::socketConnected()
{
    qDebug() << "Connection completed";
}

void TcpClient::socketDestroyed()
{
    this->socketGone = true;
}
