#include "framelesshandler.h"

#include <QEvent>
#include <QMouseEvent>
#include <QWidget>
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

FramelessHandler::FramelessHandler(QWidget *target, QObject *parent)
    : QObject(parent)
    , m_target(target)
{
    m_target->setMouseTracking(true);
    m_target->installEventFilter(this);
}

void FramelessHandler::setResizeMargin(int margin)
{
    m_resizeMargin = qMax(margin, 1);
}

int FramelessHandler::resizeMargin() const
{
    return m_resizeMargin;
}

Qt::Edges FramelessHandler::edgeAt(const QPoint &pos) const
{
    if(m_target->isMaximized())
    {
        return {};
    }

    Qt::Edges edges;
    if(pos.x() < m_resizeMargin)
    {
        edges |= Qt::LeftEdge;
    }
    if(pos.x() >= m_target->width() - m_resizeMargin)
    {
        edges |= Qt::RightEdge;
    }
    if(pos.y() < m_resizeMargin)
    {
        edges |= Qt::TopEdge;
    }
    if(pos.y() >= m_target->height() - m_resizeMargin)
    {
        edges |= Qt::BottomEdge;
    }
    return edges;
}

bool FramelessHandler::eventFilter(QObject *watched, QEvent *event)
{
    if(watched != m_target)
    {
        return QObject::eventFilter(watched, event);
    }

    switch(event->type())
    {
    case QEvent::MouseButtonPress:
    {
        auto *mouse = static_cast<QMouseEvent *>(event);
        if(mouse->button() == Qt::LeftButton && !m_target->isMaximized())
        {
            m_resizeEdges = edgeAt(mouse->pos());
            m_pressPos = globalMousePos(mouse);
            m_pressGeometry = m_target->geometry();

            QWindow *handle = m_target->windowHandle();
            const bool handled = m_resizeEdges
                ? (handle && handle->startSystemResize(m_resizeEdges))
                : (handle && handle->startSystemMove());
            if(!handled)
            {
                // 窗口系统未接管,回退手动拖动/拉伸
                m_pressed = true;
            }
        }
        break;
    }
    case QEvent::MouseMove:
    {
        auto *mouse = static_cast<QMouseEvent *>(event);
        if(m_pressed && mouse->buttons() & Qt::LeftButton)
        {
            const QPoint delta = globalMousePos(mouse) - m_pressPos;
            if(m_resizeEdges)
            {
                const int minW = qMax(m_target->minimumWidth(), 1);
                const int minH = qMax(m_target->minimumHeight(), 1);
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
                m_target->setGeometry(rect);
            }
            else
            {
                m_target->move(m_pressGeometry.topLeft() + delta);
            }
            return true;
        }

        // 未按下时根据命中边缘切换拉伸光标
        m_target->setCursor(cursorShape(edgeAt(mouse->pos())));
        break;
    }
    case QEvent::MouseButtonRelease:
        m_pressed = false;
        m_resizeEdges = {};
        break;
    case QEvent::MouseButtonDblClick:
    {
        auto *mouse = static_cast<QMouseEvent *>(event);
        if(mouse->button() == Qt::LeftButton)
        {
            m_target->isMaximized() ? m_target->showNormal() : m_target->showMaximized();
        }
        break;
    }
    default:
        break;
    }
    return QObject::eventFilter(watched, event);
}
