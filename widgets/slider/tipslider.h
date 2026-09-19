#pragma once

#include <functional>

#include "clickedslider.h"

// 悬停提示滑条:在 ClickedSlider(点击跳转)基础上,鼠标悬停/拖动时
// 在指针上方浮出气泡,显示对应位置的值(默认按毫秒格式化为 mm:ss)。
// 用于播放进度条:0..总时长(毫秒) 设 range 后直接可用
class TipSlider : public ClickedSlider
{
    Q_OBJECT
public:
    explicit TipSlider(Qt::Orientation orientation, QWidget *parent = nullptr);

    // 提示文案生成器,入参为滑条值;默认 Duration::format(ms)
    void setFormatter(std::function<QString(int)> formatter);

    QString tipText() const;   // 当前提示文案(暴露给测试)
    bool isTipVisible() const; // 气泡是否显示中

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    void updateTip(const QPoint &pos);
    int valueAt(int x) const;

    QWidget *m_bubble = nullptr;
    QString m_bubbleText;
    int m_tipValue = 0;
    std::function<QString(int)> m_formatter;
};
