#include "sidenav.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

#include "theme.h"

namespace {

constexpr int kItemHeight = 36;
constexpr int kItemPadding = 12; // 左侧文字内边距

}

SideNav::SideNav(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setMinimumWidth(160);
    connect(Theme::instance(), &Theme::themeChanged, this, [this]() { update(); });
}

int SideNav::addItem(const QString &text)
{
    m_items.append(text);
    if(m_current < 0)
    {
        m_current = 0; // 首个条目自动选中(不发信号,此时还没有监听者)
        update();
    }
    updateGeometry();
    return m_items.size() - 1;
}

int SideNav::count() const
{
    return m_items.size();
}

int SideNav::currentIndex() const
{
    return m_current;
}

void SideNav::setCurrentIndex(int index)
{
    if(index < 0 || index >= m_items.size() || index == m_current)
    {
        return;
    }
    setCurrent(index);
}

QString SideNav::itemText(int index) const
{
    return (index >= 0 && index < m_items.size()) ? m_items.at(index) : QString();
}

QSize SideNav::sizeHint() const
{
    return QSize(200, qMax(m_items.size(), 1) * kItemHeight + 8);
}

int SideNav::indexAt(const QPoint &pos) const
{
    const int index = pos.y() / kItemHeight;
    if(pos.y() < 0 || index >= m_items.size() || pos.x() < 0 || pos.x() > width())
    {
        return -1;
    }
    return index;
}

void SideNav::setCurrent(int index)
{
    m_current = index;
    update();
    emit currentChanged(index);
}

void SideNav::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    Theme *theme = Theme::instance();
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setFont(font());

    const int radius = theme->radius(Tokens::Radius::SM);
    QColor primary = theme->color(Theme::Role::Primary);
    const QColor primaryTint = QColor(primary.red(), primary.green(), primary.blue(), 26);

    for(int i = 0; i < m_items.size(); ++i)
    {
        QRect row(4, i * kItemHeight + 4, width() - 8, kItemHeight - 4);

        if(i == m_current)
        {
            // 选中:淡主色底 + 左侧主色指示条 + 主色文字
            painter.setPen(Qt::NoPen);
            painter.setBrush(primaryTint);
            painter.drawRoundedRect(row, radius, radius);
            painter.setBrush(primary);
            painter.drawRoundedRect(QRect(row.left(), row.top() + 7, 3, row.height() - 14), 1, 1);
            painter.setPen(primary);
        }
        else if(i == m_hover)
        {
            painter.setPen(Qt::NoPen);
            painter.setBrush(theme->color(Theme::Role::SurfaceVariant));
            painter.drawRoundedRect(row, radius, radius);
            painter.setPen(theme->color(Theme::Role::Text));
        }
        else
        {
            painter.setPen(theme->color(Theme::Role::TextSecondary));
        }
        painter.drawText(row.adjusted(kItemPadding, 0, -kItemPadding, 0),
                         Qt::AlignVCenter | Qt::AlignLeft, m_items.at(i));
    }
}

void SideNav::mousePressEvent(QMouseEvent *event)
{
    // 裸 QWidget 需显式接收按下,否则收不到 release
    if(event->button() == Qt::LeftButton)
    {
        m_pressed = indexAt(event->pos());
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void SideNav::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton && m_pressed >= 0)
    {
        const int released = indexAt(event->pos());
        const int pressed = m_pressed;
        m_pressed = -1;
        if(released == pressed && released >= 0)
        {
            setCurrent(released);
        }
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void SideNav::mouseMoveEvent(QMouseEvent *event)
{
    const int hover = indexAt(event->pos());
    if(hover != m_hover)
    {
        m_hover = hover;
        update();
    }
    QWidget::mouseMoveEvent(event);
}

void SideNav::leaveEvent(QEvent *event)
{
    m_hover = -1;
    m_pressed = -1;
    update();
    QWidget::leaveEvent(event);
}

void SideNav::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_Down:
        if(m_current + 1 < m_items.size())
        {
            setCurrent(m_current + 1);
        }
        event->accept();
        break;
    case Qt::Key_Up:
        if(m_current > 0)
        {
            setCurrent(m_current - 1);
        }
        event->accept();
        break;
    default:
        QWidget::keyPressEvent(event);
        break;
    }
}
