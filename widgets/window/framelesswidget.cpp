#include "framelesswidget.h"

#include <QMouseEvent>
#include <QWindow>

namespace {

Qt::CursorShape cursorShape(Qt::Edges edges)
{
    if((edges & Qt::LeftEdge && edges & Qt::TopEdge) ||
       (edges & Qt::RightEdge && edges & Qt::BottomEdge))
    {
        return Qt::SizeFDiagCursor;
    }
    if((edges & Qt::RightEdge && edges & Qt::TopEdge) ||
       (edges & Qt::LeftEdge && edges & Qt::BottomEdge))
    {
        return Qt::SizeBDiagCursor;
    }
    if(edges & (Qt::LeftEdge | Qt::RightEdge))
    {
        return Qt::SizeHorCursor;
    }
    if(edges & (Qt::TopEdge | Qt::BottomEdge))
    {
        return Qt::SizeVerCursor;
    }
    return Qt::ArrowCursor;
}

// Qt6 起才有 globalPosition();Qt5 用 globalPos()(Qt6 中仍在,仅弃用)
QPoint globalMousePos(const QMouseEvent *event)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return event->globalPosition().toPoint();
#else
    return event->globalPos();
#endif
}

}

FramelessWidget::FramelessWidget(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setMouseTracking(true);
}

void FramelessWidget::setResizeMargin(int margin)
{
    m_resizeMargin = qMax(margin, 1);
}

int FramelessWidget::resizeMargin() const
{
    return m_resizeMargin;
}

Qt::Edges FramelessWidget::edgeAt(const QPoint &pos) const
{
    if(isMaximized())
    {
        return {};
    }

    Qt::Edges edges;
    if(pos.x() < m_resizeMargin)
    {
        edges |= Qt::LeftEdge;
    }
    if(pos.x() >= width() - m_resizeMargin)
    {
        edges |= Qt::RightEdge;
    }
    if(pos.y() < m_resizeMargin)
    {
        edges |= Qt::TopEdge;
    }
    if(pos.y() >= height() - m_resizeMargin)
    {
        edges |= Qt::BottomEdge;
    }
    return edges;
}

void FramelessWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton && !isMaximized())
    {
        m_resizeEdges = edgeAt(event->pos());
        m_pressPos = globalMousePos(event);
        m_pressGeometry = geometry();

        QWindow *handle = windowHandle();
        const bool handled = m_resizeEdges
            ? (handle && handle->startSystemResize(m_resizeEdges))
            : (handle && handle->startSystemMove());
        if(!handled)
        {
            // 窗口系统未接管,回退手动拖动/拉伸
            m_pressed = true;
        }
    }
    QWidget::mousePressEvent(event);
}

void FramelessWidget::mouseMoveEvent(QMouseEvent *event)
{
    if(m_pressed && event->buttons() & Qt::LeftButton)
    {
        const QPoint delta = globalMousePos(event) - m_pressPos;
        if(m_resizeEdges)
        {
            const int minW = qMax(minimumWidth(), 1);
            const int minH = qMax(minimumHeight(), 1);
            QRect rect = m_pressGeometry;
            if(m_resizeEdges & Qt::LeftEdge)
            {
                rect.setLeft(qMin(rect.left() + delta.x(), rect.right() - minW));
            }
            if(m_resizeEdges & Qt::RightEdge)
            {
                rect.setRight(qMax(rect.right() + delta.x(), rect.left() + minW));
            }
            if(m_resizeEdges & Qt::TopEdge)
            {
                rect.setTop(qMin(rect.top() + delta.y(), rect.bottom() - minH));
            }
            if(m_resizeEdges & Qt::BottomEdge)
            {
                rect.setBottom(qMax(rect.bottom() + delta.y(), rect.top() + minH));
            }
            setGeometry(rect);
        }
        else
        {
            move(m_pressGeometry.topLeft() + delta);
        }
        return;
    }

    // 未按下时根据命中边缘切换拉伸光标
    setCursor(cursorShape(edgeAt(event->pos())));
    QWidget::mouseMoveEvent(event);
}

void FramelessWidget::mouseReleaseEvent(QMouseEvent *event)
{
    m_pressed = false;
    m_resizeEdges = {};
    QWidget::mouseReleaseEvent(event);
}

void FramelessWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        isMaximized() ? showNormal() : showMaximized();
    }
    QWidget::mouseDoubleClickEvent(event);
}
