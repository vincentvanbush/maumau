#include "gamewindow.h"
#include "ui_gamewindow.h"
#include <QPixmap>
#include <QSize>
#include <QDebug>
#include <QLabel>
#include <QMessageBox>
#include <json/json.h>
#include <json/value.h>
#include <json/writer.h>
#include <vector>
#include "cardlabel.h"

GameWindow::GameWindow(TcpClient *tcpClient, Json::Value &join_ok_msg, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GameWindow)
{
    ui->setupUi(this);
    this->tcpClient = tcpClient;

    connect(tcpClient, SIGNAL(pickCardsSignal(Json::Value &)), this, SLOT(onPickCardsMessageRecv(Json::Value &)));
    connect(tcpClient, SIGNAL(starGameSignal(Json::Value &)), this, SLOT(onStartGameMessageRecv(Json::Value &)));
    connect(tcpClient, SIGNAL(playerJoinedSignal(Json::Value &)), this, SLOT(onPlayerJoinedMessageRecv(Json::Value &)));
    connect(tcpClient, SIGNAL(playerLeftSignal(Json::Value &)), this, SLOT(onPlayerLeftMessageRecv(Json::Value&)));
    connect(tcpClient, SIGNAL(moveSignal(Json::Value&)), this, SLOT(onMoveMessageRecv(Json::Value&)));
    connect(tcpClient, SIGNAL(nextTurnSignal(Json::Value&)), this, SLOT(onNextTurnMessageRecv(Json::Value&)));
    connect(tcpClient, SIGNAL(invalidMoveSignal(Json::Value&)), this, SLOT(messageBox(Json::Value&)));

    this->mySlot = join_ok_msg["slot_number"].asInt();
    this->playerToken = join_ok_msg["player_token"].asInt();
    this->gameToken = join_ok_msg["game_token"].asInt();
    this->gameId = join_ok_msg["game_id"].asInt();

    for (int i = 0; i < 4; i++) {
        cardCounts[i] = 5;
    }

    for (unsigned i = 0; i < join_ok_msg["player_names"].size(); i++) {
        if (!join_ok_msg["player_names"][i].isNull()) {
            player_names[i] = QString(join_ok_msg["player_names"][i].asCString());
            getLabelForSlot(i)->setText(player_names[i]);
            updateCards(i);
        }
    }
    updateCards(mySlot, true);

    connect(tcpClient, SIGNAL(cannotReadySignal(Json::Value &)), this, SLOT(messageBox(Json::Value &)));
    connect(tcpClient, SIGNAL(gameEndSignal(Json::Value &)), this, SLOT(messageBox(Json::Value &)));
    connect(tcpClient, SIGNAL(playerLeftSignal()), this, SLOT(messageBox(Json::Value &)));
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

    CardLabel* lbl = new CardLabel(value, color);
    lbl->setPixmap(picture);
    lbl->setFixedSize(picture.size());

    layout->addWidget(lbl);

}

std::vector<card>* GameWindow::readCards()
{
    std::vector<card>* cards = new std::vector<card>();
    QString c = ui->lineEdit->text();

    if(!c.isEmpty()) {
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
    }
    qDebug() << "Ilosc kart 0.";

    return cards;

}

short GameWindow::readRequest()
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

void GameWindow::updateCards(int turn, bool covered) {
    QLayout *cardLayout = getCardLayoutForSlot(turn);

    QLayoutItem *child;
    while ((child = cardLayout->takeAt(0)) != 0) {
        child->widget()->hide();
        cardLayout->removeWidget(child->widget());
        delete child;
    }

    if (turn == mySlot && !covered) {
        for (unsigned i = 0; i < cardsInHand.size(); i++) {
            int value = cardsInHand[i].value;
            int color = cardsInHand[i].color;
            addCardToLayout(value, color, cardLayout);
        }
    }
    else {
        for (short i = 0; i < cardCounts[turn]; i++) {
            addCardToLayout(0, 0, cardLayout);
        }
    }
}

void GameWindow::updateTable() {
    QLayoutItem *child;
    while ((child = ui->tableCardLayout->takeAt(0)) != 0) {
        child->widget()->setParent(NULL);
        delete child;
    }
    std::vector<card> &table = cardsInTable;
    for (unsigned i = 0; i < table.size(); i++) {
        int value = table[i].value;
        int color = table[i].color;
        addCardToLayout(value, color, ui->tableCardLayout);
    }
}

QLayout *GameWindow::getCardLayoutForSlot(int slotNumber) {
    int mySlotNumber = mySlot;
    int dist = (slotNumber - mySlotNumber) % 4;
    if (dist == -3) return ui->leftCardLayout;
    if (dist == -2) return ui->topCardLayout;
    if (dist == -1) return ui->rightCardLayout;
    if (dist == 0) return ui->bottomCardLayout;
    if (dist == 1) return ui->leftCardLayout;
    if (dist == 2) return ui->topCardLayout;
    if (dist == 3) return ui->rightCardLayout;
    return nullptr;
}

QLabel *GameWindow::getLabelForSlot(int slotNumber) {
    int mySlotNumber = mySlot;
    int dist = (slotNumber - mySlotNumber) % 4;
    if (dist == -3) return ui->leftPlayerLabel;
    if (dist == -2) return ui->topPlayerLabel;
    if (dist == -1) return ui->rightPlayerLabel;
    if (dist == 0) return ui->bottomPlayerLabel;
    if (dist == 1) return ui->leftPlayerLabel;
    if (dist == 2) return ui->topPlayerLabel;
    if (dist == 3) return ui->rightPlayerLabel;
    return nullptr;
}

void GameWindow::on_moveButton_clicked()
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
        k++;
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

    tcpClient->sendMoveMessage(playerToken, gameToken, gameId, playedCardsCount, playedCards, colorRequest, valueRequest);

    //this->readCards();
}


void GameWindow::on_readyButton_clicked()
{
    tcpClient->sendReadyMessage(playerToken, gameToken, gameId);
    ui->readyButton->setDisabled(true);
}

void GameWindow::onPickCardsMessageRecv(Json::Value &msg)
{
    int cardCount = msg["count"].asInt();
    int slot = msg["slot"].asInt();

    cardCounts[slot] += cardCount;

    if (slot == mySlot) {
        for (int i = 0; i < cardCount; i++) {
            struct card card;
            card.color = msg["cards"][i]["color"].asInt();
            card.value = msg["cards"][i]["value"].asInt();
            cardsInHand.push_back(card);
        }
    }
    this->updateCards(slot);

}

void GameWindow::onStartGameMessageRecv(Json::Value &msg)
{
    int cardCount = msg["player_cards"].size();

    for (int i = 0; i < cardCount; i++) {
        struct card card;
        card.color = msg["player_cards"][i]["color"].asInt();
        card.value = msg["player_cards"][i]["value"].asInt();
        cardsInHand.push_back(card);
    }

    this->updateCards(mySlot);
    this->currentTurn = msg["turn"].asInt();
    getLabelForSlot(currentTurn)->setText("<b>" + player_names[currentTurn] + "</b>");

    struct card first_card;
    first_card.color = msg["first_card_in_stack"]["color"].asInt();
    first_card.value = msg["first_card_in_stack"]["value"].asInt();
    cardsInTable.push_back(first_card);

    updateTable();
}

void GameWindow::onPlayerJoinedMessageRecv(Json::Value &msg)
{
    QString playerName(msg["player_name"].asCString());
    int playerSlot = msg["slot_number"].asInt();
    player_names[playerSlot] = playerName;
    QLabel *playerLabel = getLabelForSlot(playerSlot);
    playerLabel->setText(playerName);

    updateCards(playerSlot);
}

void GameWindow::onPlayerLeftMessageRecv(Json::Value &msg)
{
    int playerSlot = msg["slot"].asInt();
    player_names[playerSlot] = "";
    QLabel *playerLabel = getLabelForSlot(playerSlot);
    playerLabel->setText("");

    QLayout *cardLayout = getCardLayoutForSlot(playerSlot);

    QLayoutItem *child;
    while ((child = cardLayout->takeAt(0)) != 0) {
        child->widget()->hide();
        cardLayout->removeWidget(child->widget());
        delete child;
    }
}


void GameWindow::onMoveMessageRecv(Json::Value &msg)
{
    int played_cards_count = msg["played_cards_count"].asInt();
    cardCounts[currentTurn] -= played_cards_count;
    std::vector<card> playedCards;

    for (int j = 0; j < played_cards_count; j++) {
        card playedCard;
        playedCard.color = msg["played_cards"][j]["color"].asInt();
        playedCard.value = msg["played_cards"][j]["value"].asInt();
        cardsInTable.push_back(playedCard);
        playedCards.push_back(playedCard);
    }
    updateTable();

    std::vector<card> cardsInHandCopy = cardsInHand;
    if (currentTurn == mySlot) {
        for (unsigned i = 0; i < playedCards.size(); i++) {
            card playedCard = playedCards[i];
            cardsInHand.erase(
                std::remove_if(
                    cardsInHand.begin(),
                    cardsInHand.end(),
                    [playedCard](card &handCard) {
                        return (handCard.color == playedCard.color && handCard.value == playedCard.value);
                    }),
                cardsInHand.end());
        }
    }

    int value_request = msg["value_request"].asInt();
    int color_request = msg["color_request"].asInt();

    if (value_request)
        ui->requestLabel->setText(valueString(value_request));
    else if (color_request)
        ui->requestLabel->setText(colorString(color_request));
    else
        ui->requestLabel->setText("None");

    updateCards(currentTurn);

}

QString GameWindow::valueString(int value)
{
    QString str = QString::number(value);
    if (value > 10) {
        if (value == KING) str = QString("King");
        else if (value == QUEEN) str = QString("Queen");
        else if (value == JACK) str = QString("Jack");
        else if (value == ACE) str = QString("Ace");
    }
    else if (value == 0) str = QString("None");
    return str;
}

QString GameWindow::colorString(int color)
{
    QString str = QString::number(color);
    if (color == HEART) str = QString("Hearts");
    else if (color == PIKE) str = QString("Pikes");
    else if (color == TILE) str = QString("Tiles");
    else if (color == CLOVER) str = QString("Clovers");
    else if (color == 0) str = QString("None");
    return str;
}

void GameWindow::onNextTurnMessageRecv(Json::Value &msg) {
    int turn = msg["turn"].asInt();
    int turns_for_next = msg["turns_for_next"].asInt();
    int cards_for_next = msg["cards_for_next"].asInt();

    currentTurn = turn;

    for (int i = 0; i < 3; i++) getLabelForSlot(i)->setText(player_names[i]);
    getLabelForSlot(turn)->setText("<b>" + player_names[turn] + "</b>");

    ui->turnsLabel->setText(QString::number(turns_for_next));
    ui->cardsLabel->setText(QString::number(cards_for_next));
}

void GameWindow::closeEvent(QCloseEvent *evt) {
    qDebug() << evt;
    tcpClient->sendLeaveGameMessage(playerToken, gameToken, gameId, mySlot);
    tcpClient->disconnect(this);
}

void GameWindow::messageBox(Json::Value &msg) {
    QMessageBox box;
    box.setStandardButtons(QMessageBox::Ok);
    switch (msg["msg_type"].asInt()) {
    case INVALID_MOVE:
        box.setIcon(QMessageBox::Warning);
        box.setText("This move is invalid.");
        if (!msg["cause"].isNull())
            box.setInformativeText(msg["cause"].asCString());
        break;
    case PLAYER_LEFT:
        box.setIcon(QMessageBox::Warning);
        box.setText("Player left");
        break;
    case CANNOT_READY:
        box.setIcon(QMessageBox::Warning);
        box.setText("Cannot ready");
        break;
    case GAME_END:
        box.setIcon(QMessageBox::Information);
        box.setText("Game end");
        break;
    case CANNOT_LEAVE:
        box.setIcon(QMessageBox::Warning);
        box.setText("Cannot leave");
        break;
    default:
        box.setIcon(QMessageBox::Critical);
        box.setText("Unknown message");
    }
    box.exec();
}
