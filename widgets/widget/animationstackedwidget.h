#pragma once

#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QStackedWidget>

// 带滑动切换动画的 QStackedWidget:新页从前进方向滑入,旧页反向滑出。
// 动画期间忽略新的切换请求;窗口尺寸变化时立即结束动画对齐两页
class AnimationStackedWidget : public QStackedWidget
{
    Q_OBJECT
public:
    enum Direction
    {
        Horizontal,  // 左右滑(索引增大时新页从右侧进入)
        Vertical     // 上下滑(索引增大时新页从下方进入)
    };

    explicit AnimationStackedWidget(QWidget *parent = nullptr);
    ~AnimationStackedWidget() override;

    void setDirection(Direction direction);
    Direction direction() const;
    void setDuration(int ms);  // 单次切换动画时长,<=0 视为 250
    int duration() const;
    bool isAnimating() const;

public slots:
    void setCurrentIndexAnimated(int index);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void finishAnimation();

    Direction m_direction = Horizontal;
    int m_duration = 250;
    QParallelAnimationGroup m_group;
    QPropertyAnimation m_slideOut;  // 旧页
    QPropertyAnimation m_slideIn;   // 新页
};
