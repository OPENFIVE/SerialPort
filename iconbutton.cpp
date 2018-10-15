#include "iconbutton.h"

IconButton::IconButton(QWidget *parent) : QLabel(parent)
{

}

void IconButton::mousePressEvent(QMouseEvent *ev)
{
    emit iconClicked();
    QWidget::mousePressEvent(ev);
}
