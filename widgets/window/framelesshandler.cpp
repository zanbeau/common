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

FramelessHandler::FramelessHandler(QWidget *target, QObject *parent, bool filterTarget)
    : QObject(parent)
    , m_target(target)
{
    if(filterTarget)
    {
        m_target->setMouseTracking(true);
        m_target->installEventFilter(this);
    }
}

void FramelessHandler::watch(QWidget *panel)
{
    if(m_panels.contains(panel))
    {
        return;
    }
    panel->setMouseTracking(true);
    panel->installEventFilter(this);
    m_panels.append(panel);
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
    auto *panel = qobject_cast<QWidget *>(watched);
    const bool isTarget = (watched == m_target);
    const bool isPanel = !isTarget && panel && m_panels.contains(panel);
    if(!isTarget && !isPanel)
    {
        return QObject::eventFilter(watched, event);
    }

    switch(event->type())
    {
    case QEvent::MouseButtonPress:
    {
        auto *mouse = static_cast<QMouseEvent *>(event);
        if(mouse->button() == Qt::LeftButton)
        {
            const QPoint local = isTarget ? mouse->pos()
                                          : panel->mapTo(m_target, mouse->pos());
            handlePress(local, globalMousePos(mouse));
        }
        break;
    }
    case QEvent::MouseMove:
    {
        auto *mouse = static_cast<QMouseEvent *>(event);
        const QPoint local = isTarget ? mouse->pos()
                                      : panel->mapTo(m_target, mouse->pos());
        const bool consumed = handleMove(local, globalMousePos(mouse), mouse->buttons());
        // 面板事件一律截断,避免冒泡到窗口后被窗口的 handler 再处理一遍
        if(isPanel || consumed)
        {
            return true;
        }
        break;
    }
    case QEvent::MouseButtonRelease:
        handleRelease();
        if(isPanel)
        {
            return true;
        }
        break;
    case QEvent::MouseButtonDblClick:
    {
        auto *mouse = static_cast<QMouseEvent *>(event);
        if(mouse->button() == Qt::LeftButton)
        {
            handleDoubleClick();
        }
        if(isPanel)
        {
            return true;
        }
        break;
    }
    default:
        break;
    }
    return QObject::eventFilter(watched, event);
}

void FramelessHandler::handlePress(const QPoint &localPos, const QPoint &globalPos)
{
    if(!m_target->isMaximized())
    {
        m_resizeEdges = edgeAt(localPos);
        m_pressPos = globalPos;
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
}

bool FramelessHandler::handleMove(const QPoint &localPos, const QPoint &globalPos,
                                  Qt::MouseButtons buttons)
{
    if(m_pressed && buttons & Qt::LeftButton)
    {
        const QPoint delta = globalPos - m_pressPos;
        if(m_resizeEdges)
        {
            const int minW = qMax(m_target->minimumWidth(), 1);
            const int minH = qMax(m_target->minimumHeight(), 1);
            // 未设置上限时 maximumWidth/Height 为 QWIDGETSIZE_MAX,天然不约束
            const int maxW = qMax(m_target->maximumWidth(), minW);
            const int maxH = qMax(m_target->maximumHeight(), minH);
            QRect rect = m_pressGeometry;
            if(m_resizeEdges & Qt::LeftEdge)
            {
                rect.setLeft(qBound(rect.right() - maxW, rect.left() + delta.x(), rect.right() - minW));
            }
            if(m_resizeEdges & Qt::RightEdge)
            {
                rect.setRight(qBound(rect.left() + minW, rect.right() + delta.x(), rect.left() + maxW));
            }
            if(m_resizeEdges & Qt::TopEdge)
            {
                rect.setTop(qBound(rect.bottom() - maxH, rect.top() + delta.y(), rect.bottom() - minH));
            }
            if(m_resizeEdges & Qt::BottomEdge)
            {
                rect.setBottom(qBound(rect.top() + minH, rect.bottom() + delta.y(), rect.top() + maxH));
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
    m_target->setCursor(cursorShape(edgeAt(localPos)));
    return false;
}

void FramelessHandler::handleRelease()
{
    m_pressed = false;
    m_resizeEdges = {};
}

void FramelessHandler::handleDoubleClick()
{
    m_target->isMaximized() ? m_target->showNormal() : m_target->showMaximized();
}
