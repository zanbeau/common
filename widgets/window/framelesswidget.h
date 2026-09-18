#pragma once

#include <QWidget>

// 无边框窗口基类(期待作为顶层窗口使用):
//  - 按住客户区可拖动窗口
//  - 靠近窗口边缘时按住可拉伸(八个方向)
//  - 双击客户区最大化/还原
// 拖动/拉伸优先交给窗口系统处理(原生贴边手感、Wayland 兼容),
// 窗口系统不支持时回退为手动实现
class FramelessWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FramelessWidget(QWidget *parent = nullptr);

    void setResizeMargin(int margin); // 边缘拉伸判定的宽度(逻辑像素)
    int resizeMargin() const;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    Qt::Edges edgeAt(const QPoint &pos) const;

    bool m_pressed = false;
    QPoint m_pressPos;
    QRect m_pressGeometry;
    Qt::Edges m_resizeEdges;
    int m_resizeMargin = 5;
};
