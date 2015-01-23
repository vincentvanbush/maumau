#include "cardlabel.h"
#include <QDebug>

CardLabel::CardLabel( int value, int color, QWidget * parent ) :
    QLabel(parent)

{
    this->value = value;
    this->color = color;
}

CardLabel::~CardLabel()
{
}

void CardLabel::mousePressEvent ( QMouseEvent * event )
{
    qDebug() << event;
    emit clicked();
}
