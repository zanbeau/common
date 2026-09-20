#include "iconbutton.h"

#include <QPainter>
#include <QPainterPath>
#include <QPolygonF>

#include "theme.h"

namespace {

// 在 16x16 坐标系里画实心箭头:尖点 tip,朝向 angle(度,0 = +x 方向)。
// 画刷取调用方当前画刷,画笔置空
void drawArrowHead(QPainter *p, const QPointF &tip, qreal angle)
{
    p->save();
    p->translate(tip);
    p->rotate(angle);
    QPainterPath tri;
    tri.moveTo(0, 0);
    tri.lineTo(-3.4, -2.3);
    tri.lineTo(-3.4, 2.3);
    tri.closeSubpath();
    const QBrush brush = p->brush();
    p->setPen(Qt::NoPen);
    p->setBrush(brush);
    p->drawPath(tri);
    p->restore();
}

} // namespace

IconButton::IconButton(Glyph glyph, QWidget *parent)
    : QAbstractButton(parent)
    , m_glyph(glyph)
{
    // 悬浮/按下由自绘呈现,进出场时主动重绘
    setAttribute(Qt::WA_Hover, true);
    connect(Theme::instance(), &Theme::themeChanged, this, [this]() { update(); });
}

IconButton::Glyph IconButton::glyph() const
{
    return m_glyph;
}

void IconButton::setGlyph(Glyph glyph)
{
    if(m_glyph == glyph)
    {
        return;
    }
    m_glyph = glyph;
    update();
}

int IconButton::iconSize() const
{
    return m_iconSize;
}

void IconButton::setIconSize(int size)
{
    size = qMax(8, size);
    if(m_iconSize == size)
    {
        return;
    }
    m_iconSize = size;
    updateGeometry();
    update();
}

QSize IconButton::sizeHint() const
{
    return QSize(qMax(24, m_iconSize + 16), qMax(24, m_iconSize + 16));
}

void IconButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    Theme *theme = Theme::instance();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const QRectF card = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    const qreal r = theme->radius(Tokens::Radius::MD);

    // 底色:平时透明(直接透出父窗口背景),悬浮/按下淡色底,选中态主色淡底
    QColor overlay;
    if(isEnabled() && isDown())
    {
        overlay = theme->color(Theme::Role::Text);
        overlay.setAlpha(26);
    }
    else if(isEnabled() && underMouse())
    {
        overlay = theme->color(Theme::Role::Text);
        overlay.setAlpha(16);
    }
    else if(isEnabled() && isChecked())
    {
        overlay = theme->color(Theme::Role::Primary);
        overlay.setAlpha(26);
    }
    if(overlay.isValid() && overlay.alpha() > 0)
    {
        painter.setPen(Qt::NoPen);
        painter.setBrush(overlay);
        painter.drawRoundedRect(card, r, r);
    }

    // 键盘焦点圈(Tab 导航)
    if(hasFocus())
    {
        QColor focus = theme->color(Theme::Role::Primary);
        focus.setAlpha(160);
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(focus, 1.5));
        painter.drawRoundedRect(card, r, r);
    }

    QColor iconColor = theme->color(Theme::Role::Text);
    if(!isEnabled())
    {
        iconColor = theme->color(Theme::Role::TextDisabled);
    }
    else if(isChecked())
    {
        iconColor = theme->color(Theme::Role::Primary);
    }

    const QRectF iconRect((width() - m_iconSize) / 2.0, (height() - m_iconSize) / 2.0,
                          qreal(m_iconSize), qreal(m_iconSize));
    paintGlyph(&painter, m_glyph, iconRect, iconColor);
}

void IconButton::paintGlyph(QPainter *p, Glyph glyph, const QRectF &rect, const QColor &color)
{
    if(!p || !rect.isValid() || !color.isValid())
    {
        return;
    }
    p->save();
    p->setRenderHint(QPainter::Antialiasing, true);
    // 统一缩放到 16x16 设计坐标系,线宽随图标大小等比缩放
    p->translate(rect.topLeft());
    p->scale(rect.width() / 16.0, rect.height() / 16.0);

    QPen pen(color, 1.5);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    p->setPen(pen);
    p->setBrush(Qt::NoBrush);

    switch(glyph)
    {
    case Glyph::Play:
        p->setPen(Qt::NoPen);
        p->setBrush(color);
        p->drawPolygon(QPolygonF() << QPointF(4.8, 3.2) << QPointF(12.8, 8)
                                   << QPointF(4.8, 12.8));
        break;
    case Glyph::Pause:
        p->setPen(Qt::NoPen);
        p->setBrush(color);
        p->drawRoundedRect(QRectF(4.2, 3.2, 2.8, 9.6), 1.0, 1.0);
        p->drawRoundedRect(QRectF(9.0, 3.2, 2.8, 9.6), 1.0, 1.0);
        break;
    case Glyph::Stop:
        p->setPen(Qt::NoPen);
        p->setBrush(color);
        p->drawRoundedRect(QRectF(4.0, 4.0, 8.0, 8.0), 1.5, 1.5);
        break;
    case Glyph::SkipPrevious:   // 左向三角 + 左竖条
        p->setPen(Qt::NoPen);
        p->setBrush(color);
        p->drawPolygon(QPolygonF() << QPointF(12.8, 3.5) << QPointF(6.4, 8)
                                   << QPointF(12.8, 12.5));
        p->drawRoundedRect(QRectF(3.2, 3.5, 2.4, 9.0), 1.0, 1.0);
        break;
    case Glyph::SkipNext:       // 右向三角 + 右竖条
        p->setPen(Qt::NoPen);
        p->setBrush(color);
        p->drawPolygon(QPolygonF() << QPointF(3.2, 3.5) << QPointF(9.6, 8)
                                   << QPointF(3.2, 12.5));
        p->drawRoundedRect(QRectF(10.4, 3.5, 2.4, 9.0), 1.0, 1.0);
        break;
    case Glyph::Repeat:
    {
        // 环形回路 + 上下两个流向箭头
        p->drawRoundedRect(QRectF(3.2, 4.2, 9.6, 7.6), 2.2, 2.2);
        p->setPen(Qt::NoPen);
        p->setBrush(color);
        drawArrowHead(p, QPointF(12.8, 4.2), 0);      // 上沿右行
        drawArrowHead(p, QPointF(3.2, 11.8), 180);    // 下沿左行
        break;
    }
    case Glyph::RepeatOne:
    {
        p->drawRoundedRect(QRectF(2.4, 3.6, 11.2, 8.8), 2.4, 2.4);
        p->setPen(Qt::NoPen);
        p->setBrush(color);
        drawArrowHead(p, QPointF(13.6, 3.6), 0);
        drawArrowHead(p, QPointF(2.4, 12.4), 180);
        // 中央衬线"1"
        p->setPen(pen);
        p->setBrush(Qt::NoBrush);
        p->drawLine(QPointF(8.0, 5.8), QPointF(8.0, 10.2));
        p->drawLine(QPointF(8.0, 5.8), QPointF(6.9, 6.7));
        p->drawLine(QPointF(6.4, 10.2), QPointF(9.6, 10.2));
        break;
    }
    case Glyph::Shuffle:        // 两条交叉对角线 + 右端箭头
        p->drawLine(QPointF(3.0, 4.9), QPointF(11.8, 10.4));
        p->drawLine(QPointF(3.0, 11.1), QPointF(11.8, 5.6));
        p->setPen(Qt::NoPen);
        p->setBrush(color);
        drawArrowHead(p, QPointF(13.5, 11.5), 32);
        drawArrowHead(p, QPointF(13.5, 4.5), -32);
        break;
    case Glyph::VolumeLow:
    case Glyph::VolumeHigh:
    case Glyph::VolumeMute:
    {
        // 喇叭形(实心)+ 声波弧 / 静音叉
        p->setPen(Qt::NoPen);
        p->setBrush(color);
        p->drawPolygon(QPolygonF() << QPointF(2.8, 6.1) << QPointF(5.6, 6.1)
                                   << QPointF(8.8, 3.1) << QPointF(8.8, 12.9)
                                   << QPointF(5.6, 9.9) << QPointF(2.8, 9.9));
        if(glyph == Glyph::VolumeMute)
        {
            p->setBrush(Qt::NoBrush);
            p->setPen(pen);
            p->drawLine(QPointF(10.4, 5.7), QPointF(14.2, 10.3));
            p->drawLine(QPointF(14.2, 5.7), QPointF(10.4, 10.3));
        }
        else
        {
            p->setBrush(Qt::NoBrush);
            p->setPen(pen);
            p->drawArc(QRectF(6.7, 5.4, 5.2, 5.2), -45 * 16, 90 * 16);
            if(glyph == Glyph::VolumeHigh)
            {
                p->drawArc(QRectF(4.5, 3.2, 9.6, 9.6), -45 * 16, 90 * 16);
            }
        }
        break;
    }
    case Glyph::Plus:
        p->drawLine(QPointF(8.0, 3.6), QPointF(8.0, 12.4));
        p->drawLine(QPointF(3.6, 8.0), QPointF(12.4, 8.0));
        break;
    case Glyph::Minus:
        p->drawLine(QPointF(3.6, 8.0), QPointF(12.4, 8.0));
        break;
    case Glyph::Close:
        p->drawLine(QPointF(4.2, 4.2), QPointF(11.8, 11.8));
        p->drawLine(QPointF(11.8, 4.2), QPointF(4.2, 11.8));
        break;
    case Glyph::Check:
    {
        QPainterPath check;
        check.moveTo(3.2, 8.6);
        check.lineTo(6.6, 11.8);
        check.lineTo(12.8, 4.4);
        p->drawPath(check);
        break;
    }
    case Glyph::ChevronLeft:
        p->drawLine(QPointF(10.2, 3.4), QPointF(5.6, 8.0));
        p->drawLine(QPointF(5.6, 8.0), QPointF(10.2, 12.6));
        break;
    case Glyph::ChevronRight:
        p->drawLine(QPointF(5.8, 3.4), QPointF(10.4, 8.0));
        p->drawLine(QPointF(10.4, 8.0), QPointF(5.8, 12.6));
        break;
    case Glyph::ChevronUp:
        p->drawLine(QPointF(3.4, 10.2), QPointF(8.0, 5.6));
        p->drawLine(QPointF(8.0, 5.6), QPointF(12.6, 10.2));
        break;
    case Glyph::ChevronDown:
        p->drawLine(QPointF(3.4, 5.8), QPointF(8.0, 10.4));
        p->drawLine(QPointF(8.0, 10.4), QPointF(12.6, 5.8));
        break;
    case Glyph::Ellipsis:
        p->setPen(Qt::NoPen);
        p->setBrush(color);
        p->drawEllipse(QPointF(4.0, 8.0), 1.1, 1.1);
        p->drawEllipse(QPointF(8.0, 8.0), 1.1, 1.1);
        p->drawEllipse(QPointF(12.0, 8.0), 1.1, 1.1);
        break;
    case Glyph::Heart:
    {
        QPainterPath heart;
        heart.moveTo(8.0, 13.2);
        heart.cubicTo(4.8, 10.8, 2.2, 9.0, 2.2, 6.3);
        heart.cubicTo(2.2, 4.0, 4.4, 2.6, 6.2, 2.6);
        heart.cubicTo(7.1, 2.6, 7.7, 3.1, 8.0, 3.7);
        heart.cubicTo(8.3, 3.1, 8.9, 2.6, 9.8, 2.6);
        heart.cubicTo(11.6, 2.6, 13.8, 4.0, 13.8, 6.3);
        heart.cubicTo(13.8, 9.0, 11.2, 10.8, 8.0, 13.2);
        heart.closeSubpath();
        p->setPen(Qt::NoPen);
        p->setBrush(color);
        p->drawPath(heart);
        break;
    }
    }
    p->restore();
}
