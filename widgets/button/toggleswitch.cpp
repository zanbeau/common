#include "toggleswitch.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>

#include "theme.h"

namespace {

constexpr int kTrackWidth = 44;
constexpr int kTrackHeight = 22;
constexpr int kKnobDiameter = 16;
constexpr int kTrackMargin = 3;  // 滑块与轨道边缘的留白
constexpr int kFocusPad = 3;     // 轨道外的留白,给焦点圈留位置

}

ToggleSwitch::ToggleSwitch(QWidget *parent)
    : QWidget(parent)
{
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::StrongFocus);

    m_knob.setStartValue(0.0);
    m_knob.setEndValue(1.0);
    m_knob.setDuration(Theme::instance()->duration(Tokens::Duration::Fast));
    connect(&m_knob, &QVariantAnimation::valueChanged, this,
            QOverload<>::of(&QWidget::update));
    connect(Theme::instance(), &Theme::modeChanged, this, [this]() { update(); });
}

bool ToggleSwitch::isChecked() const
{
    return m_checked;
}

void ToggleSwitch::setChecked(bool checked)
{
    if(m_checked == checked)
    {
        return;
    }
    toggle();
}

qreal ToggleSwitch::knobPos() const
{
    return m_knob.currentValue().toDouble();
}

QSize ToggleSwitch::sizeHint() const
{
    return QSize(kTrackWidth + 2 * kFocusPad, kTrackHeight + 2 * kFocusPad);
}

void ToggleSwitch::toggle()
{
    m_checked = !m_checked;
    m_knob.setDirection(m_checked ? QVariantAnimation::Forward
                                  : QVariantAnimation::Backward);
    m_knob.start();
    update();
    emit toggled(m_checked);
}

void ToggleSwitch::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    Theme *theme = Theme::instance();

    // 水平居中的圆角轨道
    const int x = (width() - kTrackWidth) / 2;
    const int y = (height() - kTrackHeight) / 2;
    const QRect track(x, y, kTrackWidth, kTrackHeight);
    const int trackRadius = kTrackHeight / 2;

    QColor trackColor = m_checked ? theme->color(Theme::Role::Primary)
                                  : theme->color(Theme::Role::BorderStrong);
    QColor knobColor = Qt::white;
    if(!isEnabled())
    {
        trackColor = theme->color(Theme::Role::SurfaceVariant);
        knobColor = theme->color(Theme::Role::TextDisabled);
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(trackColor);
    painter.drawRoundedRect(track, trackRadius, trackRadius);

    // 滑块:从左留白滑到右留白
    const int travel = kTrackWidth - kKnobDiameter - 2 * kTrackMargin;
    const qreal pos = m_knob.state() == QAbstractAnimation::Running
                          ? knobPos()
                          : (m_checked ? 1.0 : 0.0);
    const qreal knobX = x + kTrackMargin + pos * travel + kKnobDiameter / 2.0;
    const qreal knobY = y + kTrackHeight / 2.0;
    painter.setBrush(knobColor);
    painter.drawEllipse(QPointF(knobX, knobY), kKnobDiameter / 2.0, kKnobDiameter / 2.0);

    // 焦点圈:与轨道同形的药丸点线,画在轨道外留白里;
    // 若与轨道同矩形,点线会从更圆的轨道圆角外露出直角(蓝色叠蓝色只剩角可见)
    if(hasFocus())
    {
        QPen focusPen(theme->color(Theme::Role::Primary));
        focusPen.setStyle(Qt::DotLine);
        painter.setPen(focusPen);
        painter.setBrush(Qt::NoBrush);
        const QRect focusRect = track.adjusted(-kFocusPad + 1, -kFocusPad + 1,
                                               kFocusPad - 1, kFocusPad - 1);
        const int focusRadius = trackRadius + kFocusPad - 1;
        painter.drawRoundedRect(focusRect, focusRadius, focusRadius);
    }
}

void ToggleSwitch::mousePressEvent(QMouseEvent *event)
{
    // 裸 QWidget 默认忽略按下事件,必须显式接收,否则收不到后续 release
    if(event->button() == Qt::LeftButton && rect().contains(event->pos()))
    {
        m_pressedInside = true;
        event->accept();
        update();
        return;
    }
    QWidget::mousePressEvent(event);
}

void ToggleSwitch::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton && m_pressedInside)
    {
        m_pressedInside = false;
        if(rect().contains(event->pos()))
        {
            emit clicked();
            toggle();
        }
        event->accept();
        update();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void ToggleSwitch::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_Space:
    case Qt::Key_Return:
        toggle();
        event->accept();
        break;
    default:
        QWidget::keyPressEvent(event);
        break;
    }
}
