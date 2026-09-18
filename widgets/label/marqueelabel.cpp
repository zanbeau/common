#include "marqueelabel.h"

#include <QEvent>
#include <QFontMetrics>
#include <QPainter>
#include <QPaintEvent>

namespace {
constexpr int ScrollIntervalMs = 50;
// 滚出一圈后文字重新进入前留出的空隙
int scrollGap(const QRect &rect)
{
    return qMax(rect.width() / 4, 40);
}
}

MarqueeLabel::MarqueeLabel(QWidget *parent)
    : QLabel(parent)
{
    m_timer.setInterval(ScrollIntervalMs);
    connect(&m_timer, &QTimer::timeout, this, &MarqueeLabel::tick);
    m_timer.start();
}

void MarqueeLabel::setDirection(Direction direction)
{
    m_direction = direction;
}

MarqueeLabel::Direction MarqueeLabel::direction() const
{
    return m_direction;
}

bool MarqueeLabel::isScrolling() const
{
    return fontMetrics().horizontalAdvance(text()) > contentsRect().width();
}

int MarqueeLabel::offset() const
{
    return m_offset;
}

void MarqueeLabel::tick()
{
    if(!isVisible() || !isScrolling() || m_hovered)
    {
        return;
    }
    const int loop = fontMetrics().horizontalAdvance(text()) + scrollGap(contentsRect());
    m_offset = (m_offset + m_step) % loop;
    update();
}

bool MarqueeLabel::event(QEvent *event)
{
    // 用 Enter/Leave 自管 hover(部分 QPA 平台 underMouse() 不可靠)
    if(event->type() == QEvent::Enter)
    {
        m_hovered = true;
    }
    else if(event->type() == QEvent::Leave)
    {
        m_hovered = false;
    }
    return QLabel::event(event);
}

void MarqueeLabel::paintEvent(QPaintEvent *event)
{
    if(!isScrolling())
    {
        QLabel::paintEvent(event);
        return;
    }

    // 用偏移在两侧各画一份文字,形成无缝循环
    QPainter painter(this);
    const QRect rect = contentsRect();
    const int textWidth = fontMetrics().horizontalAdvance(text());
    const int loop = textWidth + scrollGap(rect);

    int x = 0;
    if(m_direction == ScrollLeft)
    {
        x = rect.width() - m_offset;  // 从右缘进入向左滚出
    }
    else
    {
        x = m_offset - textWidth;     // 从左缘进入向右滚出
    }
    // 把起点规约到 [rect.width() - loop, rect.width()),保证两侧衔接
    x = rect.width() + ((x - rect.width()) % loop + loop) % loop - loop;

    painter.drawText(QRect(x, rect.top(), textWidth, rect.height()),
                     alignment() | Qt::AlignVCenter, text());
    painter.drawText(QRect(x + loop, rect.top(), textWidth, rect.height()),
                     alignment() | Qt::AlignVCenter, text());
}
