#include "framelessshadow.h"

#include <QEvent>
#include <QPainter>
#include <QPainterPath>

#include "theme.h"

namespace {

// 阴影环宽度与最深处不透明度(Fluent/Arco 两模式同为黑色衰减,属效果参数
// 而非语义角色,故不进 Tokens 色板;亮暗只影响卡片底色与描边)
constexpr int kShadowMargin = 12;
constexpr int kShadowAlpha = 90;

} // namespace

FramelessShadow::FramelessShadow(QWidget *host)
    : QObject(host)
    , m_host(host)
{
    m_content = new QWidget(m_host);
    m_layout = new QVBoxLayout(m_content);
    m_layout->setContentsMargins(0, 0, 0, 0);

    m_host->installEventFilter(this);
    layoutContent();

    // 卡片底色/描边取自主题,亮暗与强调色变化时重绘
    connect(Theme::instance(), &Theme::themeChanged, this, [this]() { m_host->update(); });
}

QWidget *FramelessShadow::contentWidget() const
{
    return m_content;
}

QVBoxLayout *FramelessShadow::contentLayout() const
{
    return m_layout;
}

void FramelessShadow::setShadowEnabled(bool enabled)
{
    if(m_shadowEnabled == enabled)
    {
        return;
    }
    m_shadowEnabled = enabled;
    layoutContent();
    m_host->update();
}

bool FramelessShadow::shadowEnabled() const
{
    return m_shadowEnabled;
}

int FramelessShadow::shadowMargin() const
{
    return kShadowMargin;
}

void FramelessShadow::paint(QPainter *painter)
{
    // 最大化/全屏:方角满铺,无阴影无圆角(边对边才不露黑边)
    if(m_host->isMaximized() || m_host->isFullScreen())
    {
        painter->fillRect(m_host->rect(),
                          Theme::instance()->color(Theme::Role::Background));
        return;
    }

    const QRect card = m_content->geometry();
    if(m_shadowEnabled)
    {
        const QColor shadow(0, 0, 0, kShadowAlpha);
        const QColor transparent(0, 0, 0, 0);
        const int m = kShadowMargin;
        // 四边:线性渐变。四角:径向渐变(圆心 = 卡片角)。
        // 径向/线性衰减同为按距离线性,交界处数值连续,无拼缝
        QLinearGradient top(card.left(), card.top() - m, card.left(), card.top());
        top.setColorAt(0.0, transparent);
        top.setColorAt(1.0, shadow);
        painter->fillRect(card.left(), card.top() - m, card.width(), m, top);

        QLinearGradient bottom(card.left(), card.bottom() + m, card.left(), card.bottom());
        bottom.setColorAt(0.0, transparent);
        bottom.setColorAt(1.0, shadow);
        painter->fillRect(card.left(), card.bottom() + 1, card.width(), m, bottom);

        QLinearGradient left(card.left() - m, card.top(), card.left(), card.top());
        left.setColorAt(0.0, transparent);
        left.setColorAt(1.0, shadow);
        painter->fillRect(card.left() - m, card.top(), m, card.height(), left);

        QLinearGradient right(card.right() + m, card.top(), card.right(), card.top());
        right.setColorAt(0.0, transparent);
        right.setColorAt(1.0, shadow);
        painter->fillRect(card.right() + 1, card.top(), m, card.height(), right);

        painter->fillRect(card.left() - m, card.top() - m, m, m,
                          QRadialGradient(card.topLeft(), m, card.topLeft()));
        painter->fillRect(card.right() + 1 - m, card.top() - m, m, m,
                          QRadialGradient(card.topRight(), m, card.topRight()));
        painter->fillRect(card.left() - m, card.bottom() + 1 - m, m, m,
                          QRadialGradient(card.bottomLeft(), m, card.bottomLeft()));
        painter->fillRect(card.right() + 1 - m, card.bottom() + 1 - m, m, m,
                          QRadialGradient(card.bottomRight(), m, card.bottomRight()));
    }

    // 圆角卡片即内容底色:子控件不再各自铺满底色(那会把圆角盖成方角)
    const qreal radius = Theme::instance()->radius(Tokens::Radius::LG);
    QPainterPath cardPath;
    cardPath.addRoundedRect(card, radius, radius);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->fillPath(cardPath, Theme::instance()->color(Theme::Role::Background));
    painter->setPen(QPen(Theme::instance()->color(Theme::Role::Border), 1));
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(cardPath);
}

bool FramelessShadow::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == m_host)
    {
        switch(event->type())
        {
        case QEvent::Resize:
            layoutContent();
            break;
        case QEvent::WindowStateChange:
            // 最大化满铺方角、还原回到圆角 + 内缩
            layoutContent();
            m_host->update();
            break;
        default:
            break;
        }
    }
    return QObject::eventFilter(watched, event);
}

void FramelessShadow::layoutContent()
{
    const bool fullBleed = m_host->isMaximized() || m_host->isFullScreen()
                           || !m_shadowEnabled;
    const int m = kShadowMargin;
    m_content->setGeometry(fullBleed ? m_host->rect()
                                     : m_host->rect().adjusted(m, m, -m, -m));
}
