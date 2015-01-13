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
    connect(udpClient, SIGNAL(joinOKSignal(int)), this, SLOT(onJoinOKMessageRecv(int)));
    connect(udpClient, SIGNAL(gameListSignal()), this, SLOT(onGameListMessageRecv()));
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

void MainWindow::onJoinOKMessageRecv(int messType)
{
    qDebug() << "received messtype " << messType;

    QString messT = QString::number(messType);

    ui->plainTextEdit->appendPlainText(messT);
    ui->lineEdit->setText((QString) messType);
    //(const QString) messType
}

void MainWindow::onGameListMessageRecv()
{
    QString gameId;
    QString playersCount;
    QString playerName;

    for(int i=0; i<50; i++) {
        if(udpClient->gameExists[i]) {
            gameId = QString::number(udpClient->gameId[i]);
            qDebug() << "gameid" << gameId;
            playersCount = QString::number(udpClient->playersCount[i]);
            qDebug() << "playersCount" << playersCount;
            ui->plainTextEdit->appendPlainText("Game id: " + gameId + "\tplayers: " + playersCount);
            if(udpClient->started[i]) {
                ui->plainTextEdit->appendPlainText("Game in progress");
            }
            else {
                ui->plainTextEdit->appendPlainText("Game not started");
            }
            for(int j=0; j<udpClient->playersCount[i]; j++) {


                //playerName = QString::fromUtf16( (ushort*) udpClient->playerNick[i][j]);
                //playerName = QString(QLatin1String(udpClient->playerNick[i][j]));
                playerName = QString::fromUtf8(udpClient->playerNick[i][j]);
                ui->plainTextEdit->appendPlainText(playerName);
            }
        }
    }
}
