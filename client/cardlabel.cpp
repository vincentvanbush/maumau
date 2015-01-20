#include "cardlabel.h"

CardLabel::CardLabel( QWidget * parent ) :
    QLabel(parent)

{
}

CardLabel::~CardLabel()
{
}

void CardLabel::mousePressEvent ( QMouseEvent * event )
{
    emit clicked();
}
