#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    tcpClient = new TcpClient();

    // signals from interface
    connect(ui->requestGamesButton, SIGNAL(clicked()), this, SLOT(onRequestGamesButtonClicked()));
    connect(ui->joinGameButton, SIGNAL(clicked()), this, SLOT(onJoinGameButtonClicked()));
    connect(ui->readyButton, SIGNAL(clicked()), this, SLOT(onReadyButtonClicked()));
    connect(ui->leaveGameButton, SIGNAL(clicked()), this, SLOT(onLeaveGameButtonClicked()));

    //temportary
    connect(ui->sendMoveButton, SIGNAL(clicked()), this, SLOT(onSendMoveButtonClicked()));

    // reacting on messages from server
    connect(tcpClient, SIGNAL(joinOKSignal()), this, SLOT(onJoinOKMessageRecv()));
    connect(tcpClient, SIGNAL(gameListSignal()), this, SLOT(onGameListMessageRecv()));
    connect(tcpClient, SIGNAL(cannotJoinSignal()), this, SLOT(onCannotJoinMessageRecv()));
    connect(tcpClient, SIGNAL(starGameSignal()), this, SLOT(onStartGameMessageRecv()));
    connect(tcpClient, SIGNAL(playerJoinedSignal()), this, SLOT(onPlayerJoinedMessageRecv()));
    connect(tcpClient, SIGNAL(playerLeftSignal()), this, SLOT(onPlayerLeftMessageRecv()));
    connect(tcpClient, SIGNAL(moveSignal()), this, SLOT(onMoveMessageRecv()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onRequestGamesButtonClicked()
{
    ui->plainTextEdit->appendPlainText("---Sending request for games");
    tcpClient->sendRequestGamesMessage();

}

void MainWindow::onJoinGameButtonClicked()
{
    QString name = ui->setNameEdit->text();
    std::string playerName = name.toStdString();
    QString id = ui->setGameIdEdit->text();
    int gameID = id.toInt();
    tcpClient->sendJoinGameMessage(playerName, gameID);
}
void MainWindow::onReadyButtonClicked()
{
    tcpClient->sendReadyMessage();
}

void MainWindow::onLeaveGameButtonClicked()
{
    tcpClient->sendLeaveGameMessage();
}

//
// Interface reactions on received message from server.

void MainWindow::onCannotJoinMessageRecv()
{
    ui->plainTextEdit->appendPlainText("---Game full. Cannot join");
}

void MainWindow::onCannotReadyMessageRecv()
{
    ui->plainTextEdit->appendPlainText("---Cannot accept ready message");
}

void MainWindow::onCannotLeaveMessageRecv()
{
    ui->plainTextEdit->appendPlainText("---You cannot leave this game");
}

void MainWindow::onJoinOKMessageRecv()
{
    QString slotNumber = QString::number(tcpClient->slotNumber);
    QString playerToken = QString::number(tcpClient->playerToken);
    QString gameToken = QString::number(tcpClient->gameToken);


    ui->plainTextEdit->appendPlainText("Player slot in game: " + slotNumber);
    ui->plainTextEdit->appendPlainText("Player token: " + playerToken);
    ui->plainTextEdit->appendPlainText("GameToken" + gameToken);
    ui->plainTextEdit->appendPlainText("");


    int numberOfOtherPlayersInGame = 0;
    for(std::map<int, std::string*>::iterator it = tcpClient->playersAtSlots.begin(); it != tcpClient->playersAtSlots.end(); ++it) {
        if(it->second != nullptr && it->first != tcpClient->slotNumber) {
            numberOfOtherPlayersInGame ++;
        }
    }
    if(numberOfOtherPlayersInGame != 0) {
        ui->plainTextEdit->appendPlainText("There is " + QString::number(numberOfOtherPlayersInGame) + " other players in game");
    }
    else {
        ui->plainTextEdit->appendPlainText("There are no othre players in game");
    }
    for(std::map<int, std::string*>::iterator it = tcpClient->playersAtSlots.begin(); it != tcpClient->playersAtSlots.end(); ++it) {
        if(it->second != nullptr && it->first != tcpClient->slotNumber) {
            QString name = QString::fromStdString(*(it->second));
            ui->plainTextEdit->appendPlainText("Slot " + QString::number(it->first) + "\t" + name);
        }
    }



}

void MainWindow::onPlayerJoinedMessageRecv()
{
    ui->plainTextEdit->appendPlainText("New player " + QString::fromUtf8(tcpClient->nameOfLastJoinedPlayer) + " joined at slot " + QString::number(tcpClient->slotOfLastJoinedPlayer));
}

void MainWindow::onStartGameMessageRecv()
{
    QString numOfCards = QString::number(tcpClient->numberOfCardsInHand);

    ui->handTextEdit->clear();

    ui->handTextEdit->appendPlainText("Cards in hand: " + numOfCards);
    for(std::vector<card>::iterator it = tcpClient->cardsInHand.begin(); it != tcpClient->cardsInHand.end(); ++it){
        struct card card = *it;
        short color = card.color;
        short figure = card.value;
        ui->handTextEdit->appendPlainText("\t" + QString::fromStdString(this->convertCardValue(color)) + "\t" + QString::fromStdString(this->convertCardValue(figure)));
    }
    ui->handTextEdit->appendPlainText("Card in top of stack:");
    ui->handTextEdit->appendPlainText("\t" + QString::fromStdString(this->convertCardValue(tcpClient->firstCardInStack.color)) + "\t" + QString::fromStdString(this->convertCardValue(tcpClient->firstCardInStack.value)));
}

void MainWindow::onNextTurnMessageRecv()
{

}

void MainWindow::onInvalidMoveMessageRecv()
{
    ui->plainTextEdit->appendPlainText("---Invalid move. Try again\n");
}

void MainWindow::onGameEndMessageRecv()
{
    ui->plainTextEdit->appendPlainText("Game ended");
}

void MainWindow::onPlayerLeftMessageRecv()
{
    ui->plainTextEdit->appendPlainText("Player " + QString::fromUtf8(tcpClient->nameOfLastLeftPlayer) + " at slot " + QString::number(tcpClient->slotOfLastLeftPlayer) + " left");

    int numberOfOtherPlayersInGame = 0;
    for(std::map<int, std::string*>::iterator it = tcpClient->playersAtSlots.begin(); it != tcpClient->playersAtSlots.end(); ++it) {
        if(it->second != nullptr && it->first != tcpClient->slotNumber) {
            numberOfOtherPlayersInGame ++;
        }
    }
    if(numberOfOtherPlayersInGame != 0) {
        ui->plainTextEdit->appendPlainText("There is " + QString::number(numberOfOtherPlayersInGame) + " other players still in the game");
    }
    else {
        ui->plainTextEdit->appendPlainText("There are no othre players in game");
    }
}
void MainWindow::onGameListMessageRecv()
{
    QString gameId;
    QString playersCount;
    QString playerName;

    for(int i=0; i<50; i++) {
        if(tcpClient->gameExists[i]) {
            gameId = QString::number(tcpClient->gameId[i]);
            playersCount = QString::number(tcpClient->playersCount[i]);
            ui->plainTextEdit->appendPlainText("Game id: " + gameId + "\tplayers: " + playersCount);
            if(tcpClient->started[i]) {
                ui->plainTextEdit->appendPlainText("Game in progress");
            }
            else {
                ui->plainTextEdit->appendPlainText("Game not started");
            }
            for(int j=0; j<tcpClient->playersCount[i]; j++) {
                playerName = QString::fromUtf8(tcpClient->playerNick[i][j]);
                ui->plainTextEdit->appendPlainText("-" + playerName);
            }
            ui->plainTextEdit->appendPlainText("");
        }

    }
    ui->plainTextEdit->appendPlainText("");
}

void MainWindow::onMoveMessageRecv()
{
    ui->moveLogTextEdit->clear();
    ui->moveLogTextEdit->appendPlainText("Player played " + QString::number(tcpClient->playedCardsCount) + " cards");
    for(int i=0; i<tcpClient->playedCardsCount; i++) {
        ui->moveLogTextEdit->appendPlainText(QString::fromStdString(this->convertCardValue(tcpClient->playedCards[i].color)) + "\t" +
                                             QString::fromStdString(this->convertCardValue(tcpClient->playedCards[i].value)));
    }
    if(tcpClient->colorRequest != 0)
        ui->moveLogTextEdit->appendPlainText("Requested color: " + QString::fromStdString(this->convertCardValue(tcpClient->colorRequest)));
    else
        ui->moveLogTextEdit->appendPlainText("No requests about color");
    if(tcpClient->valueRequest != 0)
        ui->moveLogTextEdit->appendPlainText("Requested value: " + QString::fromStdString(this->convertCardValue(tcpClient->valueRequest)));
    else
        ui->moveLogTextEdit->appendPlainText("No requests about value");
}

std::string MainWindow::convertCardValue(int value)
{
    switch(value) {
    case 2: return "2"; break;
    case 3: return "3"; break;
    case 4: return "4"; break;
    case 5: return "5"; break;
    case 6: return "6"; break;
    case 7: return "7"; break;
    case 8: return "8"; break;
    case 9: return "9"; break;
    case 10: return "10"; break;
    case 23: return "Jack"; break;
    case 21: return "Queen"; break;
    case 20: return "King"; break;
    case 22: return "Ace"; break;
    case 30: return "Heart"; break;
    case 31: return "Tile"; break;
    case 32: return "Clover"; break;
    case 33: return "Pike"; break;
    default: return "";
    }
}

short MainWindow::readRequest()
{
    QString request = ui->requestEdit->text();
    if(request.startsWith("H")) return 30;
    else if(request.startsWith("T")) return 31;
    else if(request.startsWith("C")) return 32;
    else if(request.startsWith("P")) return 33;

    else if(request.startsWith("K")) return 20;
    else if(request.startsWith("Q")) return 21;
    else if(request.startsWith("A")) return 22;
    else if(request.startsWith("J")) return 23;

    else if(request.startsWith("2")) return 2;
    else if(request.startsWith("3")) return 3;
    else if(request.startsWith("4")) return 4;
    else if(request.startsWith("5")) return 5;
    else if(request.startsWith("6")) return 6;
    else if(request.startsWith("7")) return 7;
    else if(request.startsWith("8")) return 8;
    else if(request.startsWith("9")) return 9;
    else if(request.startsWith("10")) return 10;

    else return 0;

}

std::vector<card>* MainWindow::readCards()
{
    std::vector<card>* cards = new std::vector<card>();
    QString c = ui->playedCardsEdit->text();
    QStringList listOfCards = c.split(" ");
    qDebug() << "Ilosc kart: " + QString::number(listOfCards.length());
    foreach (QString cardString, listOfCards) {
        card cardTemp;
        short color;
        short value;
        if(cardString.startsWith("H")) color = 30;
        if(cardString.startsWith("T")) color = 31;
        if(cardString.startsWith("C")) color = 32;
        if(cardString.startsWith("P")) color = 33;


        if(cardString.contains("K")) value = 20;
        if(cardString.contains("Q")) value = 21;
        if(cardString.contains("A")) value = 22;
        if(cardString.contains("J")) value = 23;
        if(cardString.contains("2")) value = 2;
        if(cardString.contains("3")) value = 3;
        if(cardString.contains("4")) value = 4;
        if(cardString.contains("5")) value = 5;
        if(cardString.contains("6")) value = 6;
        if(cardString.contains("7")) value = 7;
        if(cardString.contains("8")) value = 8;
        if(cardString.contains("9")) value = 9;
        if(cardString.contains("10")) value = 10;

        cardTemp.color = color;
        cardTemp.value = value;
        (*cards).push_back(cardTemp);
    }




    for(std::vector<card>::iterator it = (*cards).begin(); it != (*cards).end(); ++it) {
        qDebug() << QString::fromStdString(this->convertCardValue(it->color)) + "\t" + QString::fromStdString(this->convertCardValue(it->value));
    }


    return cards;

}

void MainWindow::onSendMoveButtonClicked()
{
    short playedCardsCount = 0;
    card playedCards[4];
    for(int i=0; i<4; i++) {
        playedCards[i].color = 0;
        playedCards[i].value = 0;
    }
    short colorRequest = 0;
    short valueRequest = 0;

    std::vector<card>* cards = readCards();
    playedCardsCount = (*cards).size();

    int k = 0;
    for(std::vector<card>::iterator it = (*cards).begin(); it != (*cards).end(); ++it) {
        playedCards[k].color = it->color;
        playedCards[k].value = it->value;
    }

    short request = readRequest();
    if(request != 0) {
        if(request >= 30 && request <= 33) {
            colorRequest = request;
        }
        else {
            valueRequest = request;
        }
    }

    // VALIDATION

    tcpClient->sendMoveMessage(playedCardsCount, playedCards, colorRequest, valueRequest);



    //this->readCards();
}







