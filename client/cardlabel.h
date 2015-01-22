#ifndef CARDLABEL_H
#define CARDLABEL_H

#include <QLabel>

class CardLabel : public QLabel
{

    Q_OBJECT

public:
    explicit CardLabel( int value, int color, QWidget * parent = 0 );
    ~CardLabel();
    int value;
    int color;

signals:
    void clicked();

protected:
    void mousePressEvent ( QMouseEvent * event ) ;
};

#endif // CARDLABEL_H
