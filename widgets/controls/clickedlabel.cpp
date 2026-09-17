#include "clickedlabel.h"

#include <QMouseEvent>

ClickedLabel::ClickedLabel(const QString &text, QWidget *parent)
    : QLabel(text, parent)
{
}

void ClickedLabel::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_pressed = true;
        event->accept();
        return;
    }
    QLabel::mousePressEvent(event);
}

void ClickedLabel::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton && m_pressed)
    {
        m_pressed = false;
        if(rect().contains(event->pos()))
        {
            emit clicked();
        }
        event->accept();
        return;
    }
    QLabel::mouseReleaseEvent(event);
}
