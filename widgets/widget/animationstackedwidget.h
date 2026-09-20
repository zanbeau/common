#pragma once

#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QStackedWidget>
#include <QVariantAnimation>

// 带切换动画的 QStackedWidget:滑动(新页从前进方向滑入,旧页反向滑出)
// 或淡入淡出(动画期间两页都隐藏,由容器绘制两页的截图交叉淡化)。
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

    // 切换方式;默认 Slide 向后兼容
    enum class Transition
    {
        Slide,  // 滑动
        Fade    // 淡入淡出(旧页截图渐隐,新页截图渐显)
    };

    explicit AnimationStackedWidget(QWidget *parent = nullptr);
    ~AnimationStackedWidget() override;

    void setDirection(Direction direction);
    Direction direction() const;
    void setTransition(Transition transition);
    Transition transition() const;
    void setDuration(int ms);  // 单次切换动画时长,<=0 视为 250
    int duration() const;
    bool isAnimating() const;

public slots:
    void setCurrentIndexAnimated(int index);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    void finishAnimation();
    void finishFade();

    Direction m_direction = Horizontal;
    Transition m_transition = Transition::Slide;
    int m_duration = 250;
    QParallelAnimationGroup m_group;
    QPropertyAnimation m_slideOut;  // 旧页
    QPropertyAnimation m_slideIn;   // 新页
    QVariantAnimation m_fade;       // 淡入淡出进度 0->1
    QPixmap m_fadeOld;              // 旧页截图
    QPixmap m_fadeNew;              // 新页截图
};
