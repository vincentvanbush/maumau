#include "cardlabel.h"

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
    emit clicked();
}
