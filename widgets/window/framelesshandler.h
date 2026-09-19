#pragma once

#include <QObject>
#include <QPoint>
#include <QRect>
#include <QVector>
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
// watch(panel) 把标题栏这类"覆盖在窗口上、自身还要响应点击"的面板
// 纳入同一套行为:面板上的鼠标事件按窗口坐标处理,且不再向窗口冒泡
class FramelessHandler : public QObject
{
    Q_OBJECT
public:
    // target:要附加行为的窗口;handler 作为 target 的子对象随其销毁
    // filterTarget=false 时不过滤窗口本体,只作为 watch() 面板的载体
    // (标题栏自己创建的 watch 载体用,避免与窗口已有的 handler 重复过滤)
    explicit FramelessHandler(QWidget *target, QObject *parent = nullptr,
                              bool filterTarget = true);

    void watch(QWidget *panel); // 面板上的鼠标事件按窗口坐标参与拖动/拉伸

    void setResizeMargin(int margin); // 边缘拉伸判定的宽度(逻辑像素)
    int resizeMargin() const;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Qt::Edges edgeAt(const QPoint &pos) const;

    // localPos 已换算到窗口坐标系;globalPos 为全局位置
    void handlePress(const QPoint &localPos, const QPoint &globalPos);
    bool handleMove(const QPoint &localPos, const QPoint &globalPos,
                    Qt::MouseButtons buttons); // 返回是否已消费(拖动/拉伸中)
    void handleRelease();
    void handleDoubleClick();

    QWidget *m_target = nullptr;
    QVector<QWidget *> m_panels;
    bool m_pressed = false;
    QPoint m_pressPos;
    QRect m_pressGeometry;
    Qt::Edges m_resizeEdges;
    int m_resizeMargin = 5;
};
