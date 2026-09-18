#include "waitspinner.h"

#include <QPainter>
#include <QPaintEvent>

WaitSpinner::WaitSpinner(QWidget *parent)
    : QWidget(parent)
{
    m_angle.setStartValue(0);
    m_angle.setEndValue(360);
    m_angle.setLoopCount(-1);  // 无限循环
    connect(&m_angle, &QVariantAnimation::valueChanged, this,
            QOverload<>::of(&QWidget::update));
}

bool WaitSpinner::isSpinning() const
{
    return m_angle.state() == QAbstractAnimation::Running;
}

void WaitSpinner::setLoopDuration(int ms)
{
    m_loopDuration = (ms > 0) ? ms : 1200;
    m_angle.setDuration(m_loopDuration);
}

int WaitSpinner::loopDuration() const
{
    return m_loopDuration;
}

void WaitSpinner::setColor(const QColor &color)
{
    m_color = color;
    update();
}

QColor WaitSpinner::color() const
{
    return m_color;
}

void WaitSpinner::setLineWidth(int width)
{
    m_lineWidth = qMax(width, 1);
    update();
}

int WaitSpinner::lineWidth() const
{
    return m_lineWidth;
}

int WaitSpinner::angle() const
{
    return m_angle.currentValue().toInt();
}

void WaitSpinner::start()
{
    if(!isSpinning())
    {
        m_angle.start();
    }
    update();
}

void WaitSpinner::stop()
{
    m_angle.stop();
    update();
}

void WaitSpinner::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    if(!isSpinning())
    {
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QPen pen(m_color, m_lineWidth, Qt::SolidLine, Qt::RoundCap);
    painter.setPen(pen);

    // 弧线贴着内接正方形,留出线宽的一半避免被裁掉
    const int extent = qMin(width(), height()) - m_lineWidth;
    QRect rect((width() - extent) / 2, (height() - extent) / 2, extent, extent);
    // Qt 角度逆时针为正、16 分之一度,起始角取负让弧线沿顺时针方向转到当前角度
    painter.drawArc(rect, -angle() * 16, 100 * 16);
}
