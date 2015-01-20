#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QDialog>
#include <QLayout>
#include "../messages.h"
#include "tcpclient.h"

namespace Ui {
class GameWindow;
}

class GameWindow : public QDialog
{
    Q_OBJECT

public:
    explicit GameWindow(TcpClient *tcpClient, QWidget *parent = 0);
    ~GameWindow();

private:
    Ui::GameWindow *ui;
    void addCardToLayout(int, int, QLayout*);
    QString getCardGfxFileName(int, int);
    TcpClient *tcpClient;

private slots:
    void on_pushButton_clicked();
};

#endif // GAMEWINDOW_H
