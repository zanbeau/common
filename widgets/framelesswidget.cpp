#include "framelesswidget.h"

#include <QMouseEvent>

FramelessWidget::FramelessWidget(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
}

void FramelessWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_pressed = true;
        m_pressPos = event->globalPosition().toPoint();
    }
    QWidget::mousePressEvent(event);
}

void FramelessWidget::mouseMoveEvent(QMouseEvent *event)
{
    if(m_pressed)
    {
        const QPoint offset = event->globalPosition().toPoint() - m_pressPos;
        window()->move(window()->pos() + offset);
        m_pressPos = event->globalPosition().toPoint();
    }
    QWidget::mouseMoveEvent(event);
}

void FramelessWidget::mouseReleaseEvent(QMouseEvent *event)
{
    m_pressed = false;
    QWidget::mouseReleaseEvent(event);
}
