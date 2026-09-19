#include "rotatelabel.h"

#include <QPainter>
#include <QPainterPath>

namespace {

constexpr int kDefaultLoopMs = 6000;

}

RotateLabel::RotateLabel(QWidget *parent)
    : QWidget(parent)
{
    m_angle.setStartValue(0);
    m_angle.setEndValue(360);
    m_angle.setLoopCount(-1);  // 无限循环
    m_angle.setDuration(kDefaultLoopMs);
    connect(&m_angle, &QVariantAnimation::valueChanged, this,
            QOverload<>::of(&QWidget::update));
}

void RotateLabel::setPixmap(const QPixmap &pixmap)
{
    m_pixmap = pixmap;
    update();
}

QPixmap RotateLabel::pixmap() const
{
    return m_pixmap;
}

void RotateLabel::setRunning(bool running)
{
    if(running == isRunning())
    {
        return;
    }
    if(running)
    {
        m_angle.start();
    }
    else
    {
        m_angle.pause();
    }
    update();
}

bool RotateLabel::isRunning() const
{
    return m_angle.state() == QAbstractAnimation::Running;
}

void RotateLabel::setLoopDuration(int ms)
{
    m_angle.setDuration(ms > 0 ? ms : kDefaultLoopMs);
}

int RotateLabel::loopDuration() const
{
    return m_angle.duration();
}

void RotateLabel::setCircular(bool circular)
{
    if(m_circular == circular)
    {
        return;
    }
    m_circular = circular;
    update();
}

bool RotateLabel::isCircular() const
{
    return m_circular;
}

int RotateLabel::angle() const
{
    return m_angle.currentValue().toInt();
}

QSize RotateLabel::sizeHint() const
{
    return QSize(160, 160);
}

void RotateLabel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    if(m_pixmap.isNull())
    {
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 圆形裁剪按控件几何(不随旋转),裁出一个正圆画布
    if(m_circular)
    {
        QPainterPath clip;
        const int extent = qMin(width(), height());
        clip.addEllipse((width() - extent) / 2.0, (height() - extent) / 2.0,
                        extent, extent);
        painter.setClipPath(clip);
    }

    // 圆形模式下缩放到内接正方形,普通模式等比缩放
    const int extent = m_circular ? qMin(width(), height())
                                  : qMin(width(), height());
    const QPixmap scaled = m_pixmap.scaled(extent, extent, Qt::KeepAspectRatio,
                                           Qt::SmoothTransformation);

    painter.translate(width() / 2.0, height() / 2.0);
    painter.rotate(angle());
    painter.drawPixmap(-scaled.width() / 2.0, -scaled.height() / 2.0, scaled);
}
