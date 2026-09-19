#pragma once

#include <QPixmap>
#include <QVariantAnimation>
#include <QWidget>

// 封面渐变 Label:setPixmap() 时新旧两图交叉淡入淡出(Ant 骨架的
// 过渡动效,时长取主题 Duration::Normal);首次设置直接显示。
// 用于正在播放页的封面切换
class TransitionLabel : public QWidget
{
    Q_OBJECT
public:
    explicit TransitionLabel(QWidget *parent = nullptr);

    void setPixmap(const QPixmap &pixmap); // 触发一次渐变切换
    QPixmap pixmap() const;                // 当前(切换完成后)的图

    void setDuration(int ms);
    int duration() const;

    qreal progress() const;   // 当前过渡进度 0..1(暴露给测试)
    bool isAnimating() const;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QPixmap m_current;
    QPixmap m_next;
    QVariantAnimation m_anim;
};
