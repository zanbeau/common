#pragma once

#include <QObject>
#include <QPoint>
#include <QRect>
#include <Qt>

class QEvent;
class QMouseEvent;
class QWidget;

// 无边框窗口行为,以事件过滤器附加到任意顶层窗口:
//  - 按住客户区可拖动窗口
//  - 靠近窗口边缘时按住可拉伸(八个方向)
//  - 双击客户区最大化/还原
// 拖动/拉伸优先交给窗口系统处理(原生贴边手感、Wayland 兼容),
// 窗口系统不支持时回退为手动实现。FramelessWidget / FramelessDialog 共用。
class FramelessHandler : public QObject
{
    Q_OBJECT
public:
    // target:要附加行为的窗口;handler 作为 target 的子对象随其销毁
    explicit FramelessHandler(QWidget *target, QObject *parent = nullptr);

    void setResizeMargin(int margin); // 边缘拉伸判定的宽度(逻辑像素)
    int resizeMargin() const;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Qt::Edges edgeAt(const QPoint &pos) const;

    QWidget *m_target = nullptr;
    bool m_pressed = false;
    QPoint m_pressPos;
    QRect m_pressGeometry;
    Qt::Edges m_resizeEdges;
    int m_resizeMargin = 5;
};
