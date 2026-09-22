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

// —— 九宫格贴片渲染:按 DPR 出图,绘制时角块 1:1 贴、边条沿边缘拉伸 ——

// 边条:渐变垂直于边缘线性衰减(沿边缘方向无变化,1px 宽即可拉伸)。
// vertical = 左右边缘条(渐变沿 x);shadowAtStart = 渐变起点(x/y=0 端)是深色
QPixmap makeStrip(bool vertical, bool shadowAtStart, qreal dpr)
{
    const int m = kShadowMargin;
    QPixmap pm(vertical ? QSize(qRound(m * dpr), qMax(qRound(dpr), 1))
                        : QSize(qMax(qRound(dpr), 1), qRound(m * dpr)));
    // 新构造的 QPixmap 内存未初始化,须先清成透明——默认 SourceOver 合成会把
    // 渐变画在垃圾像素上(alpha=0 处垃圾原样保留,半透明处混入随机内容)
    pm.fill(Qt::transparent);
    pm.setDevicePixelRatio(dpr);
    QPainter p(&pm);
    QLinearGradient g(QPointF(0, 0), vertical ? QPointF(m, 0) : QPointF(0, m));
    g.setColorAt(0.0, QColor(0, 0, 0, shadowAtStart ? kShadowAlpha : 0));
    g.setColorAt(1.0, QColor(0, 0, 0, shadowAtStart ? 0 : kShadowAlpha));
    p.fillRect(QRectF(0, 0, m, m), g);
    return pm;
}

// 角块:径向渐变,圆心 = 卡片角(贴片局部坐标),半径 m——按距离线性衰减,
// 与边条在交界处数值连续、无拼缝
QPixmap makeCorner(const QPointF &cardCorner, qreal dpr)
{
    const int m = kShadowMargin;
    QPixmap pm(QSize(qRound(m * dpr), qRound(m * dpr)));
    pm.fill(Qt::transparent);   // 同 makeStrip:先清底再画,防未初始化内存混入
    pm.setDevicePixelRatio(dpr);
    QPainter p(&pm);
    QRadialGradient g(cardCorner, m, cardCorner);
    g.setColorAt(0.0, QColor(0, 0, 0, kShadowAlpha));
    g.setColorAt(1.0, QColor(0, 0, 0, 0));
    p.fillRect(QRectF(0, 0, m, m), g);
    return pm;
}

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

void FramelessShadow::ensurePatches()
{
    const qreal dpr = m_host->devicePixelRatioF();
    if(m_patchDpr == dpr)
    {
        return;
    }
    m_patchDpr = dpr;

    // 上/下边条:渐变沿 y,深色端贴卡片;左/右边条:渐变沿 x
    m_stripTop = makeStrip(false, false, dpr);
    m_stripBottom = makeStrip(false, true, dpr);
    m_stripLeft = makeStrip(true, false, dpr);
    m_stripRight = makeStrip(true, true, dpr);
    // 角块圆心 = 对应卡片角在贴片内的位置(贴片铺在角上时)
    m_cornerTL = makeCorner(QPointF(kShadowMargin, kShadowMargin), dpr);
    m_cornerTR = makeCorner(QPointF(0, kShadowMargin), dpr);
    m_cornerBL = makeCorner(QPointF(kShadowMargin, 0), dpr);
    m_cornerBR = makeCorner(QPointF(0, 0), dpr);
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
        ensurePatches();
        const int m = kShadowMargin;
        // 四边条沿边缘拉伸平铺,四角块 1:1 贴图——每帧只贴图不再栅格化渐变
        painter->drawPixmap(QRect(card.left(), card.top() - m, card.width(), m),
                            m_stripTop);
        painter->drawPixmap(QRect(card.left(), card.bottom() + 1, card.width(), m),
                            m_stripBottom);
        painter->drawPixmap(QRect(card.left() - m, card.top(), m, card.height()),
                            m_stripLeft);
        painter->drawPixmap(QRect(card.right() + 1, card.top(), m, card.height()),
                            m_stripRight);
        painter->drawPixmap(QRect(card.left() - m, card.top() - m, m, m), m_cornerTL);
        painter->drawPixmap(QRect(card.right() + 1, card.top() - m, m, m), m_cornerTR);
        painter->drawPixmap(QRect(card.left() - m, card.bottom() + 1, m, m), m_cornerBL);
        painter->drawPixmap(QRect(card.right() + 1, card.bottom() + 1, m, m), m_cornerBR);
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
