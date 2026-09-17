#include "framelesswidget.h"

#include <QMouseEvent>

FramelessWidget::FramelessWidget(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setMouseTracking(true);   // 悬停时更新边缘光标需要鼠标追踪
}

void FramelessWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_direction = resizeDirectionAt(event->pos());

        if(m_direction != ResizeDirection::None && isWindow() && !isMaximized())
        {
            m_resizing = true;
            m_resizeStartPos = event->globalPosition().toPoint();
            m_resizeStartGeometry = geometry();
        }
        else
        {
            m_pressed = true;
            m_pressPos = event->globalPosition().toPoint();
        }
    }
    QWidget::mousePressEvent(event);
}

void FramelessWidget::mouseMoveEvent(QMouseEvent *event)
{
    const QPoint globalPos = event->globalPosition().toPoint();

    if(m_resizing)
    {
        setGeometry(resizedGeometry(globalPos));
    }
    else if(m_pressed)
    {
        const QPoint offset = globalPos - m_pressPos;
        window()->move(window()->pos() + offset);
        m_pressPos = globalPos;
    }
    else
    {
        updateResizeCursor(event->pos());
    }
    QWidget::mouseMoveEvent(event);
}

void FramelessWidget::mouseReleaseEvent(QMouseEvent *event)
{
    m_pressed = false;
    m_resizing = false;
    m_direction = ResizeDirection::None;
    QWidget::mouseReleaseEvent(event);
}

FramelessWidget::ResizeDirection FramelessWidget::resizeDirectionAt(const QPoint &pos) const
{
    const bool nearLeft = pos.x() < ResizeMargin;
    const bool nearRight = pos.x() >= width() - ResizeMargin;
    const bool nearTop = pos.y() < ResizeMargin;
    const bool nearBottom = pos.y() >= height() - ResizeMargin;

    if(nearTop && nearLeft)         return ResizeDirection::TopLeft;
    if(nearTop && nearRight)        return ResizeDirection::TopRight;
    if(nearBottom && nearLeft)      return ResizeDirection::BottomLeft;
    if(nearBottom && nearRight)     return ResizeDirection::BottomRight;
    if(nearTop)                     return ResizeDirection::Top;
    if(nearBottom)                  return ResizeDirection::Bottom;
    if(nearLeft)                    return ResizeDirection::Left;
    if(nearRight)                   return ResizeDirection::Right;
    return ResizeDirection::None;
}

void FramelessWidget::updateResizeCursor(const QPoint &pos)
{
    if(!isWindow() || isMaximized())
    {
        setCursor(Qt::ArrowCursor);
        return;
    }

    Qt::CursorShape shape = Qt::ArrowCursor;
    switch(resizeDirectionAt(pos))
    {
    case ResizeDirection::Top:
    case ResizeDirection::Bottom:
        shape = Qt::SizeVerCursor;
        break;
    case ResizeDirection::Left:
    case ResizeDirection::Right:
        shape = Qt::SizeHorCursor;
        break;
    case ResizeDirection::TopLeft:
    case ResizeDirection::BottomRight:
        shape = Qt::SizeFDiagCursor;
        break;
    case ResizeDirection::TopRight:
    case ResizeDirection::BottomLeft:
        shape = Qt::SizeBDiagCursor;
        break;
    default:
        break;
    }
    setCursor(shape);
}

QRect FramelessWidget::resizedGeometry(const QPoint &globalPos) const
{
    const QPoint delta = globalPos - m_resizeStartPos;
    QRect rect = m_resizeStartGeometry;
    const int minWidth = qMax(minimumWidth(), 100);
    const int minHeight = qMax(minimumHeight(), 80);

    // 左/上边缘收缩时保持不小于最小尺寸（右下角坐标随之修正）
    if(m_direction == ResizeDirection::Left || m_direction == ResizeDirection::TopLeft ||
       m_direction == ResizeDirection::BottomLeft)
    {
        rect.setLeft(qMin(rect.left() + delta.x(), rect.right() - minWidth + 1));
    }
    if(m_direction == ResizeDirection::Right || m_direction == ResizeDirection::TopRight ||
       m_direction == ResizeDirection::BottomRight)
    {
        rect.setRight(qMax(rect.right() + delta.x(), rect.left() + minWidth - 1));
    }
    if(m_direction == ResizeDirection::Top || m_direction == ResizeDirection::TopLeft ||
       m_direction == ResizeDirection::TopRight)
    {
        rect.setTop(qMin(rect.top() + delta.y(), rect.bottom() - minHeight + 1));
    }
    if(m_direction == ResizeDirection::Bottom || m_direction == ResizeDirection::BottomLeft ||
       m_direction == ResizeDirection::BottomRight)
    {
        rect.setBottom(qMax(rect.bottom() + delta.y(), rect.top() + minHeight - 1));
    }
    return rect;
}
