#ifndef CARDLABEL_H
#define CARDLABEL_H

#include <QLabel>

class CardLabel : public QLabel
{

    Q_OBJECT

public:
    explicit CardLabel( QWidget * parent = 0 );
    ~CardLabel();

signals:
    void clicked();

protected:
    void mousePressEvent ( QMouseEvent * event ) ;
};

#endif // CARDLABEL_H
