#include "transitionlabel.h"

#include <QPainter>

#include "theme.h"

namespace {

// 等比缩放到控件内并居中
QRect centeredRect(const QSize &size, const QPixmap &pixmap)
{
    const QPixmap scaled = pixmap.scaled(size, Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation);
    return QRect((size.width() - scaled.width()) / 2,
                 (size.height() - scaled.height()) / 2,
                 scaled.width(), scaled.height());
}

}

TransitionLabel::TransitionLabel(QWidget *parent)
    : QWidget(parent)
{
    m_anim.setStartValue(0.0);
    m_anim.setEndValue(1.0);
    m_anim.setDuration(Theme::instance()->duration(Tokens::Duration::Normal));
    connect(&m_anim, &QVariantAnimation::valueChanged, this,
            QOverload<>::of(&QWidget::update));
    connect(&m_anim, &QVariantAnimation::finished, this, [this]() {
        m_current = m_next;
        update();
    });
}

void TransitionLabel::setPixmap(const QPixmap &pixmap)
{
    if(m_current.isNull())
    {
        m_current = pixmap; // 首次设置直接显示,不动效
        update();
        return;
    }

    // 上一场还在播就先定格到上一张,再开新一场
    if(m_anim.state() == QAbstractAnimation::Running)
    {
        m_anim.stop();
        m_current = m_next;
    }
    m_next = pixmap;
    m_anim.start();
}

QPixmap TransitionLabel::pixmap() const
{
    return m_current;
}

void TransitionLabel::setDuration(int ms)
{
    m_anim.setDuration(qMax(ms, 1));
}

int TransitionLabel::duration() const
{
    return m_anim.duration();
}

qreal TransitionLabel::progress() const
{
    return m_anim.currentValue().toDouble();
}

bool TransitionLabel::isAnimating() const
{
    return m_anim.state() == QAbstractAnimation::Running;
}

void TransitionLabel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    if(m_current.isNull())
    {
        return;
    }

    QPainter painter(this);
    const qreal t = isAnimating() ? progress() : 1.0;

    if(isAnimating() && !m_next.isNull())
    {
        // 交叉淡化:旧图渐隐、新图渐显
        painter.setOpacity(1.0 - t);
        painter.drawPixmap(centeredRect(size(), m_current), m_current);
        painter.setOpacity(t);
        painter.drawPixmap(centeredRect(size(), m_next), m_next);
        painter.setOpacity(1.0);
        return;
    }

    painter.drawPixmap(centeredRect(size(), m_current), m_current);
}
