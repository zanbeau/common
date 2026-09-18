#include "animationstackedwidget.h"

#include <QResizeEvent>

AnimationStackedWidget::AnimationStackedWidget(QWidget *parent)
    : QStackedWidget(parent)
{
    m_slideOut.setPropertyName("pos");
    m_slideIn.setPropertyName("pos");
    m_slideOut.setEasingCurve(QEasingCurve::OutCubic);
    m_slideIn.setEasingCurve(QEasingCurve::OutCubic);
    m_group.addAnimation(&m_slideOut);
    m_group.addAnimation(&m_slideIn);
    connect(&m_group, &QParallelAnimationGroup::finished, this,
            &AnimationStackedWidget::finishAnimation);
}

AnimationStackedWidget::~AnimationStackedWidget()
{
    // 析构期间动画组销毁链可能再触发 finished;先解除目标并断开回调,
    // 避免回跳 finishAnimation 访问已销毁的成员/页面
    m_group.disconnect(this);
    m_group.stop();
    m_slideOut.setTargetObject(nullptr);
    m_slideIn.setTargetObject(nullptr);
}

void AnimationStackedWidget::setDirection(Direction direction)
{
    m_direction = direction;
}

AnimationStackedWidget::Direction AnimationStackedWidget::direction() const
{
    return m_direction;
}

void AnimationStackedWidget::setDuration(int ms)
{
    m_duration = (ms > 0) ? ms : 250;
    m_slideOut.setDuration(m_duration);
    m_slideIn.setDuration(m_duration);
}

int AnimationStackedWidget::duration() const
{
    return m_duration;
}

bool AnimationStackedWidget::isAnimating() const
{
    return m_group.state() == QAbstractAnimation::Running;
}

void AnimationStackedWidget::setCurrentIndexAnimated(int index)
{
    if(isAnimating() || index < 0 || index >= count() || index == currentIndex())
    {
        return;
    }

    // 先把当前页记下并切换,再手动把旧页显示出来参与动画
    QWidget *oldPage = currentWidget();
    const int oldIndex = currentIndex();
    setCurrentIndex(index);
    QWidget *newPage = currentWidget();
    if(!oldPage || !newPage)
    {
        return;
    }

    const bool forward = (index > oldIndex);
    QPoint off;  // 旧页滑出的终点偏移 = 新页滑入的起点偏移
    if(m_direction == Horizontal)
    {
        off = QPoint(forward ? -width() : width(), 0);
    }
    else
    {
        off = QPoint(0, forward ? -height() : height());
    }

    newPage->resize(size());
    oldPage->resize(size());
    m_slideOut.setTargetObject(oldPage);
    m_slideIn.setTargetObject(newPage);
    m_slideOut.setStartValue(QPoint(0, 0));
    m_slideOut.setEndValue(off);
    m_slideIn.setStartValue(-off);
    m_slideIn.setEndValue(QPoint(0, 0));

    oldPage->show();
    m_group.start();
}

void AnimationStackedWidget::finishAnimation()
{
    // 旧页归位并交还给 QStackedWidget 管理(仅当前页可见)
    if(QWidget *page = qobject_cast<QWidget *>(m_slideOut.targetObject()))
    {
        page->move(0, 0);
        page->hide();
    }
    if(QWidget *page = qobject_cast<QWidget *>(m_slideIn.targetObject()))
    {
        page->move(0, 0);
        page->show();
    }
    m_slideOut.setTargetObject(nullptr);
    m_slideIn.setTargetObject(nullptr);
}

void AnimationStackedWidget::resizeEvent(QResizeEvent *event)
{
    QStackedWidget::resizeEvent(event);
    if(isAnimating())
    {
        m_group.stop();  // stop 会触发 finished -> finishAnimation 对齐两页
    }
    if(currentWidget())
    {
        currentWidget()->resize(event->size());
    }
}
