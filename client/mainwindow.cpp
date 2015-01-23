#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QInputDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    bool ok;
    QString ipAddr = QInputDialog::getText(this, "Connect to server", "Please enter server IP address (default is 127.0.0.1)", QLineEdit::Normal, QString(), &ok);
    if (!ok) ipAddr = "127.0.0.1";

    tcpClient = new TcpClient(ipAddr);

    for(int i=0; i<50; i++) {
        this->gamesIds[i] = nullptr;
        for(int j=0; j<4; j++)
            this->playerNames[i][j] = nullptr;
        this->gameStarted[i] = nullptr;
    }
    //this->fillGamesTable();

    // signals from interface
    connect(ui->requestGamesButton, SIGNAL(clicked()), this, SLOT(onRequestGamesButtonClicked()));
    connect(ui->joinGameButton, SIGNAL(clicked()), this, SLOT(onJoinGameButtonClicked()));
    connect(ui->newGameButton, SIGNAL(clicked()), this, SLOT(onNewGameButtonClicked()));

    // reacting on messages from server
    connect(tcpClient, SIGNAL(cannotJoinSignal(Json::Value &)), this, SLOT(onCannotJoinMessageRecv(Json::Value &)));
    connect(tcpClient, SIGNAL(joinOKSignal(Json::Value &)), this, SLOT(onJoinOKMessageRecv(Json::Value &)));
    connect(tcpClient, SIGNAL(gameListSignal(Json::Value &)), this, SLOT(onGameListMessageRecv(Json::Value &)));
    connect(tcpClient, SIGNAL(socketErrorSignal(QAbstractSocket::SocketError)), this, SLOT(onSocketErrorSignal(QAbstractSocket::SocketError)));

    tcpClient->sendRequestGamesMessage();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// methods handling signals from interface e.g. clicking buttons
void MainWindow::onRequestGamesButtonClicked()
{
    tcpClient->sendRequestGamesMessage();
}

void MainWindow::onNewGameButtonClicked()
{
    QString name = ui->setNameEdit->text();
    std::string playerName = name.toStdString();

    int gameID = -1;
    if(playerName != "") {
        tcpClient->sendJoinGameMessage(playerName, gameID);
    }
    else {
        qDebug() << "You need to type your nick";
    }
}

void MainWindow::onJoinGameButtonClicked()
{
    QString name = ui->setNameEdit->text();
    std::string playerName = name.toStdString();

    int gameID;

    QModelIndexList ind = ui->gamesListView->selectionModel()->selectedRows();
    if(ind.size() != 0) {
        QModelIndex index = ui->gamesListView->model()->index(ind.at(0).row(),0,QModelIndex());
        if(playerName != "") {
            gameID = ui->gamesListView->model()->data(index).toString().toInt();
            if(playersCount[gameID] < 4) {
                tcpClient->sendJoinGameMessage(playerName, gameID);
            }
            else {
                qDebug() << "Too many players in this game";
            }
        }
        else {
            qDebug() << "Nie wpisałeś imienia";
        }
    }
    else {
        qDebug() << "Nie wybrałeś gry do dołączenia";
    }

}


// Interface reactions on received message from server.

void MainWindow::onCannotJoinMessageRecv(Json::Value &msg)
{
    qDebug() << "Cannot join!";
}

void MainWindow::onJoinOKMessageRecv(Json::Value &msg)
{
    GameWindow *gameWindow = new GameWindow(this->tcpClient, msg, this);
    gameWindow->show();
}

void MainWindow::onGameListMessageRecv(Json::Value &msg)
{
    Json::Value &games = msg["games"];
    //QStandardItemModel model;

    if(!games.isNull()) {
        for(unsigned i=0; i<games.size(); i++) {
            gameExists[i] = !games[i].isNull();
            qDebug("Game %d exists? %d", i, gameExists[i]);

            if (!gameExists[i]) continue;
            gameId[i] = games[i]["id"].asInt();
            playersCount[i] = games[i]["players_count"].asInt();
            for(int j=0; j<4; j++) {
                if (games[i]["player_nicks"][j].isNull()) strcpy(playerNick[i][j], "");
                else strcpy(playerNick[i][j], games[i]["player_nicks"][j].asCString());
            }
            started[i] = games[i]["started"].asBool();
        }
        for (unsigned i=games.size(); i < 50; i++) {
            gameExists[i] = false;
        }
    }
    for(int i=0; i<50; i++) {
        if(this->gamesIds[i] != nullptr) {
            delete this->gamesIds[i];
            this->gamesIds[i] = nullptr;
        }
        for(int j=0; j<4; j++) {
            if(this->playerNames[i][j] != nullptr) {
                delete this->playerNames[i][j];
                this->playerNames[i][j] = nullptr;
            }
        }
        if(this->gameStarted[i] != nullptr) {
            delete this->gameStarted[i];
            this->gameStarted[i] = nullptr;
        }
    }

    this->horizontalHeader.clear();
    model.clear();

    this->horizontalHeader.append("Game id");
    this->horizontalHeader.append("Player 1");
    this->horizontalHeader.append("Player 2");
    this->horizontalHeader.append("Player 3");
    this->horizontalHeader.append("Player 4");
    this->horizontalHeader.append("Game status");

    model.index(1,1,model.index(0,0));
    model.setHorizontalHeaderLabels(this->horizontalHeader);

    int rowCount = 0;
    for(int i=0; i<50; i++) {

        if(this->gameExists[i]) {
            qDebug() << "ID: " << i;
            this->gamesIds[i] = new QStandardItem(QString(QString::number(gameId[i])));
            model.setItem(rowCount,0,this->gamesIds[i]);

            for(int j=0; j<4; j++) {
                this->playerNames[i][j] = new QStandardItem(QString(QString::fromUtf8(playerNick[i][j])));
                model.setItem(rowCount,j+1,this->playerNames[i][j]);
            }

            if(started[i])
                this->gameStarted[i] = new QStandardItem(QString("In progress"));
            else
                this->gameStarted[i] = new QStandardItem(QString("Not started"));
            model.setItem(rowCount,5,this->gameStarted[i]);
            rowCount++;
        }
        else {
            model.removeRow(i);
        }
    }

    ui->gamesListView->setModel(&model);
    ui->gamesListView->resizeRowsToContents();
    ui->gamesListView->resizeColumnsToContents();
}

// Helpers
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

void MainWindow::onSocketErrorSignal(QAbstractSocket::SocketError err) {
    QString errName;
    QDebug(&errName) << err;
    this->statusBar()->showMessage("Connection error: " + errName);
}
