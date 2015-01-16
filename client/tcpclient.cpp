#include "tcpclient.h"


TcpClient::TcpClient()
{
    this->serverIPAddress = "127.0.0.1";
    this->serverPort = 1234;

    for(int i=0; i<4; i++) {
        this->playersAtSlots[i] = nullptr;
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


void TcpClient::readMessage()
{
    struct game_msg gameMessage;
    QHostAddress addr (serverIPAddress);

    tcpSocket->read((char *) &gameMessage, (qint64) sizeof(gameMessage));

    int messageType = gameMessage.msg_type;

    switch(messageType) {
    case CANNOT_JOIN:
        this->cannotJoinSignal();
        break;
    case CANNOT_READY:
        this->cannotReadySignal();
        break;
    case CANNOT_LEAVE:
        this->cannotLeaveSignal();
    case JOIN_OK:
        this->joinOKSignalHandle(gameMessage.message.join_ok);
        this->joinOKSignal();
        break;
    case PLAYER_JOINED:
        this->playerJoinedSignalHandle(gameMessage.message.player_joined);
        this->playerJoinedSignal();
        break;
    case START_GAME:
        this->startGameSignalHandle(gameMessage.message.start_game);
        this->starGameSignal();
        break;
    case NEXT_TURN:
//        this->nextTurnSignalHandle(gameMessage.message.next_turn);
//        this->nextTurnSignal();
        break;
    case INVALID_MOVE:
        this->invalidMoveSignal();
        break;
    case GAME_END:
        this->gameEndSignalHandle(gameMessage.message.game_end);
        this->gameEndSignal();
        break;
    case PLAYER_LEFT:
        this->playerLeftSignalHandle(gameMessage.message.player_left);
        this->playerLeftSignal();
        break;
    case GAME_LIST:
        this->gameListSignalHandle(gameMessage.message.game_list);
        this->gameListSignal();
    case MOVE:
        this->moveSignalHandle(gameMessage.message.move);
        this->moveSignal();
        break;
    }

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
    struct game_msg gameMessage;
    gameMessage.msg_type = 0;

    const char* playerNameChar = playerName.c_str();

    strcpy(this->playerName, playerNameChar);
    this->gameIdentifier = gameID;

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

void TcpClient::sendReadyMessage()
{
    struct game_msg gameMessage;
    gameMessage.msg_type = 1;
    gameMessage.token = this->playerToken;
    gameMessage.game_token = this->gameToken;
    gameMessage.game_id = this->gameIdentifier;

    //gameMessage.message.ready.dummy = 0;
    // is it necessery to send also proper struct if it's content is dummy?

    this->tcpSocket->write((const char*) &gameMessage, (qint64) sizeof(gameMessage));
}

void TcpClient::sendLeaveGameMessage()
{
    struct game_msg gameMessage;
    gameMessage.msg_type = 2;
    gameMessage.token = this->playerToken;
    gameMessage.game_token = this->gameToken;
    gameMessage.game_id = this->gameIdentifier;
    gameMessage.message.leave_game.slot = this->slotNumber;



    // updating info about this game
    this->playersCount[this->gameIdentifier]--;
    for(int i=0; i<4; i++) {
        this->playerNick[this->gameIdentifier][i][0] = '\0';
    }
    this->started[this->gameIdentifier] = false;


    this->tcpSocket->write((const char*) &gameMessage, (qint64) sizeof(gameMessage));



}

void TcpClient::sendMoveMessage(short playedCardsCount, card* playedCards, short colorRequest, short valueRequest)
{
    struct game_msg gameMessage;
    struct move_msg move;



    move.played_cards_count = playedCardsCount;
    for(int i=0; i<4; i++) {
        move.played_cards[i] = playedCards[i];
    }
    move.color_request = colorRequest;
    move.value_request = valueRequest;

    gameMessage.msg_type = 11;
    gameMessage.token = this->playerToken;
    gameMessage.game_token = this->gameToken;
    gameMessage.game_id = this->gameIdentifier;
    gameMessage.message.move = move;

    this->tcpSocket->write((const char*) &gameMessage, (qint64) sizeof(gameMessage));
}


// methods handling communicates from server
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
    // TODO
    // future change in protocole; additional info about slots and name players currently in game


    std::string *name = new std::string(this->playerName);
    this->playersAtSlots[this->slotNumber] = name;

    // adding other players in game to map
    for(int i=0; i<4; i++) {
        if(joinOK.slot_taken[i] && i != this->slotNumber) {
            std::string *otherPlayerName = new std::string(joinOK.player_name[i]);
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

void TcpClient::startGameSignalHandle(struct start_game_msg startGame)
{

    for(int i=0; i<5; i++) {
        this->cardsInHand.push_back(startGame.player_cards[i]);
    }
    this->numberOfCardsInHand = 5;

    this->cardsInStack.push_back(startGame.first_card_in_stack);
    this->firstCardInStack = startGame.first_card_in_stack;
    this->moveAtSlot = startGame.turn;

    this->started[this->gameIdentifier] = true;
}

void TcpClient::playerJoinedSignalHandle(struct player_joined_msg playerJoined)
{

    char playerName[30];
    short slotNumber = playerJoined.slot_number;
    strcpy(playerName, playerJoined.player_name);

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

void TcpClient::gameEndSignalHandle(struct game_end_msg gameEnd)
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

void TcpClient::playerLeftSignalHandle(struct player_left_msg playerLeft)
{
    this->slotOfLastLeftPlayer = playerLeft.slot;
    const char *playerName = (*(this->playersAtSlots[this->slotOfLastLeftPlayer])).c_str();
    strcpy(this->nameOfLastLeftPlayer, playerName);

    this->playersAtSlots[this->slotOfLastLeftPlayer] = nullptr;

    // updating info about this game
    this->playersCount[this->gameIdentifier]--;
    this->playerNick[this->gameIdentifier][this->slotOfLastLeftPlayer][0] = '\0';

}

void TcpClient::moveSignalHandle(struct move_msg move)
{
    this->playedCardsCount = move.played_cards_count;
    for(int i=0; i<this->playedCardsCount; i++) {
        this->playedCards[i] = move.played_cards[i];
        this->cardsInStack.push_back(move.played_cards[i]);
    }

    card c;
    c.color = this->cardsInStack.end()->color;
    c.value = this->cardsInStack.end()->value;
    //this->firstCardInStack = this->playedCards[this->playedCardsCount-1];
    this->firstCardInStack.color = c.color;
    this->firstCardInStack.value = c.value;


    this->colorRequest = move.color_request;
    this->valueRequest = move.value_request;

}

void TcpClient::nextTurnSignalHandle(struct next_turn_msg nextTurn)
{
    this->turn = nextTurn.turn;
    this->cards_for_next = nextTurn.cards_for_next;
    this->turns_for_next = nextTurn.turns_for_next;
}

void TcpClient::pickCardsSignalHandle(struct pick_cards_msg pickCards)
{
    this->slot = pickCards.slot;
    this->count = pickCards.count;
    if(this->slot == this->slotNumber) {
        for(int i=0; i<this->count; i++) {
            this->cardsInHand.push_back(pickCards.cards[i]);
        }
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


