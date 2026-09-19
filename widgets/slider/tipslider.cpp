#include "tipslider.h"

#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyle>

#include "duration.h"
#include "theme.h"

namespace {

// 跟随指针的气泡:无边框顶层小窗,自绘圆角底 + 文字
class TipBubble : public QWidget
{
public:
    explicit TipBubble()
        : QWidget(nullptr, Qt::ToolTip | Qt::FramelessWindowHint)
    {
        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_ShowWithoutActivating);
    }

    void setText(const QString &text)
    {
        m_text = text;
        QFontMetrics metrics(font());
        const int w = metrics.horizontalAdvance(text) + 20;
        const int h = metrics.height() + 10;
        resize(qMax(w, 40), h);
        update();
    }

private:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event)
        Theme *theme = Theme::instance();
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        QPainterPath card;
        card.addRoundedRect(rect().adjusted(0, 0, -1, -1), 4, 4);
        painter.fillPath(card, theme->mode() == Theme::Mode::Dark
                                   ? theme->color(Theme::Role::SurfaceVariant)
                                   : QColor(0x29, 0x29, 0x2E));
        painter.setPen(theme->color(Theme::Role::TextOnPrimary));
        painter.setFont(font());
        painter.drawText(rect(), Qt::AlignCenter, m_text);
    }

    QString m_text;
};

} // namespace

TipSlider::TipSlider(Qt::Orientation orientation, QWidget *parent)
    : ClickedSlider(orientation, parent)
{
    setMouseTracking(true);
    m_formatter = [](int value) { return Duration::format(value); };
}

void TipSlider::setFormatter(std::function<QString(int)> formatter)
{
    m_formatter = formatter ? std::move(formatter)
                            : std::function<QString(int)>(
                                  [](int value) { return Duration::format(value); });
    if(isTipVisible())
    {
        m_bubbleText = m_formatter(m_tipValue);
        // m_bubble 由本文件的 TipBubble 创建,向下转型安全
        static_cast<TipBubble *>(m_bubble)->setText(m_bubbleText);
    }
}

QString TipSlider::tipText() const
{
    return m_bubbleText;
}

bool TipSlider::isTipVisible() const
{
    return m_bubble && m_bubble->isVisible();
}

int TipSlider::valueAt(int x) const
{
    const int span = qMax(width() - 1, 1);
    return QStyle::sliderValueFromPosition(minimum(), maximum(),
                                           qBound(0, x, span), span);
}

void TipSlider::updateTip(const QPoint &pos)
{
    if(orientation() != Qt::Horizontal)
    {
        return; // 竖直滑条暂不提示
    }

    if(!m_bubble)
    {
        m_bubble = new TipBubble;
    }

    m_tipValue = valueAt(pos.x());
    m_bubbleText = m_formatter(m_tipValue);
    // m_bubble 由本文件的 TipBubble 创建,向下转型安全
    static_cast<TipBubble *>(m_bubble)->setText(m_bubbleText);

    // 气泡悬在指针上方并水平居中,避免超出滑条左右边界
    const QPoint global = mapToGlobal(pos);
    const int bubbleX = global.x() - m_bubble->width() / 2;
    m_bubble->move(qMax(0, bubbleX), global.y() - m_bubble->height() - 8);
    if(!m_bubble->isVisible())
    {
        m_bubble->show();
    }
}

void TipSlider::mouseMoveEvent(QMouseEvent *event)
{
    updateTip(event->pos());
    ClickedSlider::mouseMoveEvent(event);
}

void TipSlider::leaveEvent(QEvent *event)
{
    if(m_bubble)
    {
        m_bubble->hide();
    }
    ClickedSlider::leaveEvent(event);
}

void TipSlider::hideEvent(QHideEvent *event)
{
    // 滑条本身隐藏时(如切换页面)把气泡一并收掉
    if(m_bubble)
    {
        m_bubble->hide();
    }
    ClickedSlider::hideEvent(event);
}
