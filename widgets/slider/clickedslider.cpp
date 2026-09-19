#include "clickedslider.h"

#include <QMouseEvent>
#include <QStyle>
#include <QStyleOptionSlider>

ClickedSlider::ClickedSlider(Qt::Orientation orientation, QWidget *parent)
    : QSlider(orientation, parent)
{
}

ClickedSlider::ClickedSlider(QWidget *parent)
    : QSlider(Qt::Horizontal, parent)
{
}

void ClickedSlider::mousePressEvent(QMouseEvent *event)
{
    // 空量程时显式接收按下:QSlider 对空量程会 ignore,事件冒泡到
    // 无边框父窗口会被当成"空白区按下"而拖动窗口
    if(event->button() == Qt::LeftButton && maximum() == minimum())
    {
        event->accept();
        return;
    }
    if(event->button() == Qt::LeftButton && maximum() > minimum())
    {
        QStyleOptionSlider opt;
        initStyleOption(&opt);
        const QRect groove = style()->subControlRect(QStyle::CC_Slider, &opt,
                                                     QStyle::SC_SliderGroove, this);
        const QRect handle = style()->subControlRect(QStyle::CC_Slider, &opt,
                                                     QStyle::SC_SliderHandle, this);
        const bool horizontal = (orientation() == Qt::Horizontal);
        const int span = horizontal ? groove.width() - handle.width()
                                    : groove.height() - handle.height();
        if(span <= 0)
        {
            QSlider::mousePressEvent(event);
            return;
        }

        // 点击位置换算成手柄可走的像素行程,再映射为值
        const QPoint click = event->pos();
        int pos = horizontal ? click.x() - groove.x() - handle.width() / 2
                             : click.y() - groove.y() - handle.height() / 2;
        pos = qBound(0, pos, span);
        setValue(QStyle::sliderValueFromPosition(minimum(), maximum(), pos, span,
                                                 invertedAppearance()));
        emit clicked();

        // 把事件伪造成落在新的手柄中心再交给基类,按住拖动得以继续
        QStyleOptionSlider synced;
        initStyleOption(&synced);
        const QRect newHandle = style()->subControlRect(QStyle::CC_Slider, &synced,
                                                        QStyle::SC_SliderHandle, this);
        QMouseEvent press(QEvent::MouseButtonPress, QPointF(newHandle.center()),
                          QPointF(mapToGlobal(newHandle.center())),
                          event->button(), event->buttons(), event->modifiers());
        QSlider::mousePressEvent(&press);
        event->accept();
        return;
    }
    QSlider::mousePressEvent(event);
}
