#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    udpClient = new UdpClient();

    connect(ui->requestGamesButton, SIGNAL(clicked()), this, SLOT(onRequestGamesButtonClicked()));
    connect(ui->joinGameButton, SIGNAL(clicked()), this, SLOT(onJoinGameButtonClicked()));
    connect(udpClient, SIGNAL(joinOKSignal(int)), this, SLOT(onJoinOKMessageRecv(int)));
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
    std::string playerName;
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
