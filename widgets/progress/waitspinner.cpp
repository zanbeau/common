#include "waitspinner.h"

#include <QPainter>
#include <QPaintEvent>

#include "theme.h"

WaitSpinner::WaitSpinner(QWidget *parent)
    : QWidget(parent)
{
    m_color = Theme::instance()->color(Theme::Role::Primary);
    m_angle.setStartValue(0);
    m_angle.setEndValue(360);
    m_angle.setLoopCount(-1);  // 无限循环
    setLoopDuration(m_loopDuration);  // 必须显式设置,否则 QVariantAnimation 默认 250ms
    connect(&m_angle, &QVariantAnimation::valueChanged, this,
            QOverload<>::of(&QWidget::update));
    // 未设自定义色时跟随主题换色
    connect(Theme::instance(), &Theme::modeChanged, this, [this]() {
        if(!m_customColor)
        {
            m_color = Theme::instance()->color(Theme::Role::Primary);
            update();
        }
    });
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
    m_customColor = true;
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

QSize WaitSpinner::sizeHint() const
{
    return QSize(24, 24);
}

void WaitSpinner::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    if(!isSpinning())
    {
        return;
    }

    // 控件比线宽还小时不绘制,避免生成负尺寸的矩形
    const int extent = qMin(width(), height()) - m_lineWidth;
    if(extent <= 0)
    {
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QPen pen(m_color, m_lineWidth, Qt::SolidLine, Qt::RoundCap);
    painter.setPen(pen);

    QRect rect((width() - extent) / 2, (height() - extent) / 2, extent, extent);
    // Qt 角度逆时针为正、16 分之一度,起始角取负让弧线沿顺时针方向转到当前角度
    painter.drawArc(rect, -angle() * 16, 100 * 16);
}
