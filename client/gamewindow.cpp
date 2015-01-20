#include "gamewindow.h"
#include "ui_gamewindow.h"
#include <QPixmap>
#include <QSize>
#include <QDebug>
#include <QLabel>
#include "cardlabel.h"

GameWindow::GameWindow(TcpClient *tcpClient, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GameWindow)
{
    ui->setupUi(this);
    this->tcpClient = tcpClient;

    /*
    connect(tcpClient, SIGNAL(cannotReadySignal()), this, SLOT(onCannotReadyMessageRecv()));
    connect(tcpClient, SIGNAL(cannotLeaveSignal()), this, SLOT(onCannotLeaveMessageRecv()));
    connect(tcpClient, SIGNAL(playerJoinedSignal()), this, SLOT(onPlayerJoinedMessageRecv()));
    connect(tcpClient, SIGNAL(starGameSignal()), this, SLOT(onStartGameMessageRecv()));
    connect(tcpClient, SIGNAL(nextTurnSignal()), this, SLOT(onNextTurnMessageRecv()));
    connect(tcpClient, SIGNAL(pickCardsSignal()), this, SLOT(onPickCardsMessageRecv()));
    connect(tcpClient, SIGNAL(invalidMoveSignal()), this, SLOT(onInvalidMoveMessageRecv()));
    connect(tcpClient, SIGNAL(gameEndSignal()), this, SLOT(onGameEndMessageRecv()));
    connect(tcpClient, SIGNAL(playerLeftSignal()), this, SLOT(onPlayerLeftMessageRecv()));
    connect(tcpClient, SIGNAL(moveSignal()), this, SLOT(onMoveMessageRecv()));*/
}

GameWindow::~GameWindow()
{
    delete ui;
}

QString GameWindow::getCardGfxFileName(int value, int color) {
    QString filename(":/images/");
    if (value == 0) {
        return QString(":/images/reverse.png");
    }

    if (value < KING) filename += QString::number(value);
    else {
        if (value == KING) filename += "king";
        else if (value == QUEEN) filename += "queen";
        else if (value == ACE) filename += "ace";
        else if (value == JACK) filename += "jack";
    }
    filename += "_of_";
    if (color == HEART) filename += "hearts";
    else if (color == TILE) filename += "diamonds";
    else if (color == CLOVER) filename += "clubs";
    else if (color == PIKE) filename += "spades";
    filename += ".png";

    return filename;
}

void GameWindow::addCardToLayout(int value, int color, QLayout *layout) {
    QString filename = getCardGfxFileName(value, color);
    QPixmap picture(filename);

    CardLabel* lbl = new CardLabel();
    lbl->setPixmap(picture);
    lbl->setFixedSize(picture.size());
    layout->addWidget(lbl);
}

void GameWindow::on_pushButton_clicked()
{
    addCardToLayout(rand() % 9 + 2, rand() % 4 + HEART, ui->bottomCardLayout);
    addCardToLayout(0, rand() % 4 + HEART, ui->topCardLayout);
    addCardToLayout(0, rand() % 4 + HEART, ui->leftCardLayout);
    addCardToLayout(0, rand() % 4 + HEART, ui->rightCardLayout);
    addCardToLayout(rand() % 9 + 2, rand() % 4 + HEART, ui->tableCardLayout);
}
