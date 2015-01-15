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

    // reacting on messages from server
    connect(tcpClient, SIGNAL(joinOKSignal()), this, SLOT(onJoinOKMessageRecv()));
    connect(tcpClient, SIGNAL(gameListSignal()), this, SLOT(onGameListMessageRecv()));
    connect(tcpClient, SIGNAL(cannotJoinSignal()), this, SLOT(onCannotJoinMessageRecv()));
    connect(tcpClient, SIGNAL(starGameSignal()), this, SLOT(onStartGameMessageRecv()));
    connect(tcpClient, SIGNAL(playerJoinedSignal()), this, SLOT(onPlayerJoinedMessageRecv()));
    connect(tcpClient, SIGNAL(playerLeftSignal()), this, SLOT(onPlayerLeftMessageRecv()));
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

    ui->plainTextEdit->clear();
    ui->plainTextEdit->appendPlainText("Cards in hand: " + numOfCards);
    for(std::vector<card>::iterator it = tcpClient->cardsInHand.begin(); it != tcpClient->cardsInHand.end(); ++it){
        struct card card = *it;
        short color = card.color;
        short figure = card.value;
        ui->plainTextEdit->appendPlainText("\t" + QString::number(color) + "\t" + QString::number(figure));
    }
    ui->plainTextEdit->appendPlainText("Card in top of stack:");
    ui->plainTextEdit->appendPlainText("\t" + QString::number(tcpClient->firstCardInStack.color) + "\t" + QString::number(tcpClient->firstCardInStack.value));
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







