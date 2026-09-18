#pragma once

#include <QLabel>
#include <QTimer>

// 文字超宽时自动横向滚动的 QLabel(滚动歌名等);hover 时暂停。
// 文字不超宽时按普通 QLabel 显示。offset() 主要供测试观察滚动进度
class MarqueeLabel : public QLabel
{
    Q_OBJECT
public:
    enum Direction
    {
        ScrollLeft,  // 向左滚出
        ScrollRight  // 向右滚出
    };

    explicit MarqueeLabel(QWidget *parent = nullptr);

    void setDirection(Direction direction);
    Direction direction() const;

    // 文字当前是否超宽(即处于滚动模式,与 hover 暂停无关)
    bool isScrolling() const;
    int offset() const;

protected:
    void paintEvent(QPaintEvent *event) override;
    bool event(QEvent *event) override;

private:
    void tick();

    Direction m_direction = ScrollLeft;
    int m_offset = 0;          // 一个完整循环的像素行程
    int m_step = 2;            // 每次滚动步进的像素
    bool m_hovered = false;    // 自管 hover 状态(underMouse 在部分平台不可靠)
    QTimer m_timer;
};
