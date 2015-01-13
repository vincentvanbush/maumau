#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    udpClient = new UdpClient();

    // signals from interface
    connect(ui->requestGamesButton, SIGNAL(clicked()), this, SLOT(onRequestGamesButtonClicked()));
    connect(ui->joinGameButton, SIGNAL(clicked()), this, SLOT(onJoinGameButtonClicked()));

    // reacting on messages from server
    connect(udpClient, SIGNAL(joinOKSignal()), this, SLOT(onJoinOKMessageRecv()));
    connect(udpClient, SIGNAL(gameListSignal()), this, SLOT(onGameListMessageRecv()));
    connect(udpClient, SIGNAL(cannotJoinSignal()), this, SLOT(onCannotJoinMessageRecv()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onRequestGamesButtonClicked()
{
    ui->lineEdit->setText("Sending request for games");
    udpClient->sendRequestGamesMessage();

}

void MainWindow::onJoinGameButtonClicked()
{
    std::string playerName = "Mat";
    udpClient->sendJoinGameMessage(playerName);
}

void MainWindow::onJoinOKMessageRecv()
{
    //qDebug() << "received messtype " << messType;

    //QString messT = QString::number(messType);

    //ui->plainTextEdit->appendPlainText(messT);
    //ui->lineEdit->setText((QString) messType);

    QString slotNumber = QString::number(udpClient->slotNumber);
    QString playerToken = QString::number(udpClient->playerToken);
    QString gameToken = QString::number(udpClient->gameToken);


    ui->plainTextEdit->appendPlainText("Player slot in game: " + slotNumber);
    ui->plainTextEdit->appendPlainText("Player token: " + playerToken);
    ui->plainTextEdit->appendPlainText("gameToken" + gameToken);

}

void MainWindow::onGameListMessageRecv()
{
    QString gameId;
    QString playersCount;
    QString playerName;

    for(int i=0; i<50; i++) {
        if(udpClient->gameExists[i]) {
            gameId = QString::number(udpClient->gameId[i]);
            playersCount = QString::number(udpClient->playersCount[i]);
            ui->plainTextEdit->appendPlainText("Game id: " + gameId + "\tplayers: " + playersCount);
            if(udpClient->started[i]) {
                ui->plainTextEdit->appendPlainText("Game in progress");
            }
            else {
                ui->plainTextEdit->appendPlainText("Game not started");
            }
            for(int j=0; j<udpClient->playersCount[i]; j++) {
                playerName = QString::fromUtf8(udpClient->playerNick[i][j]);
                ui->plainTextEdit->appendPlainText(playerName);
            }
        }
    }
}

void MainWindow::onCannotJoinMessageRecv()
{
    ui->plainTextEdit->appendPlainText("Game full. Cannot join");
}
