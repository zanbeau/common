#include "animationstackedwidget.h"

#include <QEasingCurve>
#include <QPainter>
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

    m_fade.setStartValue(0.0);
    m_fade.setEndValue(1.0);
    m_fade.setEasingCurve(QEasingCurve::OutCubic);
    connect(&m_fade, &QVariantAnimation::valueChanged, this, [this]() { update(); });
    connect(&m_fade, &QVariantAnimation::finished, this, &AnimationStackedWidget::finishFade);
}

AnimationStackedWidget::~AnimationStackedWidget()
{
    // 析构期间动画销毁链可能再触发 finished;先解除目标并断开回调,
    // 避免回跳 finishAnimation/finishFade 访问已销毁的成员/页面
    m_group.disconnect(this);
    m_group.stop();
    m_fade.disconnect(this);
    m_fade.stop();
    m_slideOut.setTargetObject(nullptr);
    m_slideIn.setTargetObject(nullptr);
    m_fadeOld = QPixmap();
    m_fadeNew = QPixmap();
    // 淡入淡出中当前页是被藏起的,恢复可见交还 QStackedWidget 管理
    if(currentWidget())
    {
        currentWidget()->show();
    }
}

void AnimationStackedWidget::setDirection(Direction direction)
{
    m_direction = direction;
}

AnimationStackedWidget::Direction AnimationStackedWidget::direction() const
{
    return m_direction;
}

void AnimationStackedWidget::setTransition(Transition transition)
{
    m_transition = transition;
}

AnimationStackedWidget::Transition AnimationStackedWidget::transition() const
{
    return m_transition;
}

void AnimationStackedWidget::setDuration(int ms)
{
    m_duration = (ms > 0) ? ms : 250;
    m_slideOut.setDuration(m_duration);
    m_slideIn.setDuration(m_duration);
    m_fade.setDuration(m_duration);
}

int AnimationStackedWidget::duration() const
{
    return m_duration;
}

bool AnimationStackedWidget::isAnimating() const
{
    return m_group.state() == QAbstractAnimation::Running
           || m_fade.state() == QAbstractAnimation::Running;
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
    if(m_transition == Transition::Fade)
    {
        if(!oldPage)
        {
            return;
        }
        oldPage->resize(size());
        m_fadeOld = oldPage->grab();
        m_fadeNew = QPixmap();  // 清掉上一轮残留,防动画首帧画出旧图
        setCurrentIndex(index);
        QWidget *newPage = currentWidget();
        if(!newPage || m_fadeOld.isNull())
        {
            return;
        }
        newPage->resize(size());
        m_fadeNew = newPage->grab();
        if(m_fadeNew.isNull())
        {
            return;
        }
        // 两页都藏起:子控件永远画在容器自绘之上,动画期间由容器绘制两张截图
        oldPage->hide();
        newPage->hide();
        m_fade.start();
        return;
    }

    QWidget *newPage = nullptr;
    setCurrentIndex(index);
    newPage = currentWidget();
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

void AnimationStackedWidget::finishFade()
{
    m_fadeOld = QPixmap();
    m_fadeNew = QPixmap();
    // 当前页在淡入淡出期间是被藏起的,结束即恢复
    if(QWidget *page = currentWidget())
    {
        if(!page->isVisible())
        {
            page->show();
        }
    }
    update();
}

void AnimationStackedWidget::paintEvent(QPaintEvent *event)
{
    QStackedWidget::paintEvent(event);
    if(m_fade.state() != QAbstractAnimation::Running || m_fadeNew.isNull())
    {
        return;
    }
    // 新页截图全程不透明打底,旧页截图按进度渐隐盖上——
    // 对称的交叉淡化会在中段透出容器底色,打底可避免
    QPainter painter(this);
    painter.drawPixmap(0, 0, m_fadeNew);
    const qreal t = m_fade.currentValue().toReal();
    if(t < 1.0)
    {
        painter.setOpacity(1.0 - t);
        painter.drawPixmap(0, 0, m_fadeOld);
    }
}

void AnimationStackedWidget::resizeEvent(QResizeEvent *event)
{
    QStackedWidget::resizeEvent(event);
    // 中途停止的动画不会发出 finished(Qt 仅在自然到点时发),
    // 必须显式收尾对齐两页;两个 finish 均幂等,重复调用无害
    if(isAnimating())
    {
        m_group.stop();
        m_fade.stop();
        finishAnimation();
        finishFade();
    }
    if(currentWidget())
    {
        currentWidget()->resize(event->size());
    }
}
