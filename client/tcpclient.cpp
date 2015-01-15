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
        break;
    }

}

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

void TcpClient::socketError(QAbstractSocket::SocketError err)
{
    qDebug() << "Problem with socket";
    qDebug() << err;
}

void TcpClient::socketConnected()
{
    qDebug() << "Connection completed";
}


