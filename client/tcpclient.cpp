#include "tcpclient.h"
#include <json/json.h>

TcpClient::TcpClient()
{
    this->serverIPAddress = "192.168.0.27";
    this->serverPort = 1234;

    for(int i=0; i<4; i++) {
        this->playersAtSlots[i] = nullptr;
    }
    this->turn = 0;
    this->turnsForNext = 0;
    this->cardsForNext = 0;


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
    void * gm = malloc(10 * sizeof(struct game_msg));
    QHostAddress addr (serverIPAddress);

    do { // do while there is something to read, as after one message there may be another
        int msg_len;
        tcpSocket->read((char*)&msg_len, (qint64) sizeof(int));

        Json::Value gameMessage = recvMessage(msg_len);
        int messageType = gameMessage["msg_type"].asInt();

        qDebug() << "Message type: " + QString::number(messageType);

        switch(messageType) {
        case CANNOT_JOIN:
            this->cannotJoinSignal();
            break;
        case CANNOT_READY:
            this->cannotReadySignal();
            break;
        case CANNOT_LEAVE:
            this->cannotLeaveSignal();
            break;
        case JOIN_OK:
            this->joinOKSignalHandle(gameMessage);
            this->joinOKSignal();
            break;
        case PLAYER_JOINED:
            this->playerJoinedSignalHandle(gameMessage);
            this->playerJoinedSignal();
            break;
        case START_GAME:
            this->startGameSignalHandle(gameMessage);
            this->starGameSignal();
            break;
        case NEXT_TURN:
            this->nextTurnSignalHandle(gameMessage);
            this->nextTurnSignal();
            break;
        case PICK_CARDS:
            this->pickCardsSignalHandle(gameMessage);
            this->pickCardsSignal();
            break;
        case INVALID_MOVE:
            this->invalidMoveSignal();
            break;
        case GAME_END:
            this->gameEndSignalHandle(gameMessage);
            this->gameEndSignal();
            break;
        case PLAYER_LEFT:
            this->playerLeftSignalHandle(gameMessage);
            this->playerLeftSignal();
            break;
        case GAME_LIST:
            this->gameListSignalHandle(gameMessage);
            this->gameListSignal();
            break;
        case MOVE:
            this->moveSignalHandle(gameMessage);
            this->moveSignal();
            break;
        }
    } while (tcpSocket->bytesAvailable());

}

void TcpClient::updateHandAfterMove(short playedCardsCount, card* cards)
{
    card c;
    if(playedCardsCount > 0) {
        for(int i=0; i<playedCardsCount; i++) {
            for(std::vector<card>::iterator it = this->cardsInHand.begin(); it != this->cardsInHand.end(); ++it) {
                if(it->color == cards[i].color && it->value == cards[i].value) {

                    c.color = it->color;
                    c.value = it->value;
                    this->cardsInStack.push_back(c);
                    this->cardsInHand.erase(it);
                    break;
                }
            }
        }

        this->firstCardInStack.color = c.color;
        this->firstCardInStack.value = c.value;
        this->numberOfCardsInHand = this->cardsInHand.size();
    }
}



// methods sending communicates to server
void TcpClient::sendJoinGameMessage(std::string playerName, int gameID)
{
    Json::Value join_game_msg;
    join_game_msg["msg_type"] = JOIN_GAME;
    join_game_msg["player_name"] = playerName.c_str();
    join_game_msg["game_id"] = gameID;
    this->gameIdentifier = gameID;

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
    Json::Value &games = msg["games"];
    for(int i=0; i<games.size(); i++) {
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
    // TODO
    // future change in protocole; additional info about slots and name players currently in game


    std::string *name = new std::string(this->playerName);
    this->playersAtSlots[this->slotNumber] = name;

    // adding other players in game to map
    for(int i=0; i<4; i++) {
        if(!joinOK["player_names"][i].isNull() && i != this->slotNumber) {
            std::string *otherPlayerName = new std::string(joinOK["player_names"][i].asString());
            this->playersAtSlots[i] = otherPlayerName;
        }
    }

    int numberOfPlayersInGame = 0;
    for(std::map<int, std::string*>::iterator it = this->playersAtSlots.begin(); it != this->playersAtSlots.end(); ++it) {
        if(it->second != nullptr)
            numberOfPlayersInGame++;
    }
    qDebug() << "There is/are " << QString::number(numberOfPlayersInGame) << " players in game";

    for(std::map<int, std::string*>::iterator it = this->playersAtSlots.begin(); it != this->playersAtSlots.end(); ++it) {
        if(it->second != nullptr) {
            std::string n = *(it->second);
            qDebug() << QString::number(it->first) << " " << QString::fromStdString(n);
        }
    }


    // updating info about this game
    this->gameExists[this->gameIdentifier] = true;
    this->gameId[this->gameIdentifier] = this->gameIdentifier;
    this->playersCount[this->gameIdentifier]++;
    strcpy(this->playerNick[this->gameIdentifier][this->slotNumber], this->playerName);



}

void TcpClient::startGameSignalHandle(Json::Value& startGame)
{

    for(int i=0; i<5; i++) {
        struct card card;
        card.value = startGame["player_cards"][i]["value"].asInt();
        card.color = startGame["player_cards"][i]["color"].asInt();
        this->cardsInHand.push_back(card);
    }
    this->numberOfCardsInHand = 5;

    struct card first_card_in_stack;
    first_card_in_stack.value = startGame["first_card_in_stack"]["value"].asInt();
    first_card_in_stack.color = startGame["first_card_in_stack"]["color"].asInt();

    this->cardsInStack.push_back(first_card_in_stack);
    this->firstCardInStack = first_card_in_stack;
    this->moveAtSlot = startGame["turn"].asInt();

    this->started[this->gameIdentifier] = true;
}

void TcpClient::playerJoinedSignalHandle(Json::Value& playerJoined)
{

    char playerName[30];
    short slotNumber = playerJoined["slot_number"].asInt();
    strcpy(playerName, playerJoined["player_name"].asCString());

    this->slotOfLastJoinedPlayer = slotNumber;
    strcpy(this->nameOfLastJoinedPlayer, playerName);

    std::string *name = new std::string(playerName);
    this->playersAtSlots[slotNumber] = name;

    int numberOfPlayersInGame = 0;
    for(std::map<int, std::string*>::iterator it = this->playersAtSlots.begin(); it != this->playersAtSlots.end(); ++it) {
        if(it->second != nullptr) {
            numberOfPlayersInGame++;
        }
    }

    qDebug() << "There is/are " << QString::number(numberOfPlayersInGame) << " players in game";
    for(std::map<int, std::string*>::iterator it = this->playersAtSlots.begin(); it != this->playersAtSlots.end(); ++it) {
        if(it->second != nullptr) {
            std::string n = *(it->second);
            qDebug() << QString::number(it->first) << " " << QString::fromStdString(n);
        }

    }

    // updating info about this game
    this->playersCount[this->gameIdentifier]++;
    strcpy(this->playerNick[this->gameIdentifier][slotNumber], playerName);
}

void TcpClient::gameEndSignalHandle(Json::Value& gameEnd)
{
    this->numberOfCardsInHand = 0;
    this->cardsInHand.clear();
    for(std::map<int, std::string*>::iterator it = playersAtSlots.begin(); it != playersAtSlots.end(); ++it) {
        it->second = nullptr;
    }

    this->gameIdentifier = 0;
    this->nameOfLastJoinedPlayer[0] = '\0';


    // updating info about this game
    this->gameExists[this->gameIdentifier] = false;
    this->playersCount[this->gameIdentifier] = 0;
    for(int i=0; i<4; i++) {
        this->playerNick[this->gameIdentifier][i][0] = '\0';
    }
    this->started[this->gameIdentifier] = false;
}

void TcpClient::playerLeftSignalHandle(Json::Value& playerLeft)
{
    this->slotOfLastLeftPlayer = playerLeft["slot"].asInt();
    const char *playerName = (*(this->playersAtSlots[this->slotOfLastLeftPlayer])).c_str();
    strcpy(this->nameOfLastLeftPlayer, playerName);

    this->playersAtSlots[this->slotOfLastLeftPlayer] = nullptr;

    // updating info about this game
    this->playersCount[this->gameIdentifier]--;
    this->playerNick[this->gameIdentifier][this->slotOfLastLeftPlayer][0] = '\0';

}

void TcpClient::moveSignalHandle(Json::Value& move)
{
    this->playedCardsCount = move["played_cards_count"].asInt();
    for(int i=0; i<this->playedCardsCount; i++) {
        struct card card;
        card.value = move["played_cards"][i]["value"].asInt();
        card.color = move["played_cards"][i]["color"].asInt();

        this->playedCards[i] = card;
        this->cardsInStack.push_back(card);
    }

    card c;
    c.color = this->cardsInStack.end()->color;
    c.value = this->cardsInStack.end()->value;
    //this->firstCardInStack = this->playedCards[this->playedCardsCount-1];
    this->firstCardInStack.color = c.color;
    this->firstCardInStack.value = c.value;

    this->colorRequest = move["color_request"].asInt();
    this->valueRequest = move["value_request"].asInt();
}

void TcpClient::nextTurnSignalHandle(Json::Value& nextTurn)
{
    this->turn = nextTurn["turn"].asInt();
    this->cardsForNext = nextTurn["cards_for_next"].asInt();
    this->turnsForNext= nextTurn["turns_for_next"].asInt();
}

void TcpClient::pickCardsSignalHandle(Json::Value &pickCards)
{
    this->slot = pickCards["slot"].asInt();
    this->count = pickCards["count"].asInt();
    if(this->slot == this->slotNumber) {
        for(int i=0; i<this->count; i++) {
            struct card card;
            card.value = pickCards["cards"][i]["value"].asInt();
            card.color = pickCards["cards"][i]["color"].asInt();

            this->cardsInHand.push_back(card);
        }
        this->numberOfCardsInHand = this->cardsInHand.size();

        // temporary version
//        int i=0;
//        while(pickCards.cards[i].color != 0) {
//            this->cardsInHand.push_back(pickCards.cards[i]);
//            i++;
//        }
//        this->numberOfCardsInHand = this->cardsInHand.size();
    }
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


