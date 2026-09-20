#pragma once

#include <QWidget>

class FramelessHandler;

// 无边框窗口基类(期待作为顶层窗口使用):
//  - 窗口边缘按住可拉伸(八个方向)
//  - 拖动与双击最大化只在 watch 的面板上(如 TitleBar;v0.10.0 起),
//    内容区按下不认领——否则列表空白处/控件缝隙会误触发窗口拖动
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
