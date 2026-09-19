#pragma once

#include <QWidget>

class FramelessHandler;

// 无边框窗口基类(期待作为顶层窗口使用):
//  - 按住客户区可拖动窗口
//  - 靠近窗口边缘时按住可拉伸(八个方向)
//  - 双击客户区最大化/还原
// 行为由 FramelessHandler 提供(优先窗口系统的 startSystemMove/Resize,
// 不支持时回退手动实现),FramelessDialog 与此类共用同一套行为
class FramelessWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FramelessWidget(QWidget *parent = nullptr);

    void setResizeMargin(int margin); // 边缘拉伸判定的宽度(逻辑像素)
    int resizeMargin() const;

private:
    FramelessHandler *m_handler = nullptr;
};
