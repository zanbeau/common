#pragma once

#include <QSize>
#include <QWidget>

class FramelessHandler;
class FramelessShadow;
class QVBoxLayout;

// 无边框窗口基类(期待作为顶层窗口使用),自带阴影 + 圆角卡片外观:
//  - 业务布局放在 contentLayout() 上(不要直接 new 布局到窗口),
//    阴影内缩、最大化满铺由窗口自动维护
//  - 窗口边缘按住可拉伸(八个方向,命中区 = 阴影环)
//  - 拖动与双击最大化只在 watch 的面板上(如 TitleBar;v0.10.0 起),
//    内容区按下不认领——否则列表空白处/控件缝隙会误触发窗口拖动
// 行为由 FramelessHandler 提供(优先窗口系统的 startSystemMove/Resize,
// 不支持时回退手动实现),FramelessDialog 与此类共用同一套行为;
// 阴影/圆角由 FramelessShadow 提供(setShadowEnabled 可关)
class FramelessWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FramelessWidget(QWidget *parent = nullptr);

    QWidget *contentWidget() const;      // 业务布局的宿主(透明容器)
    QVBoxLayout *contentLayout() const;  // contentWidget 上的 0 边距布局

    void setShadowEnabled(bool enabled); // 关 = 无阴影无内缩(圆角仍在)
    bool shadowEnabled() const;
    int shadowMargin() const;            // 阴影环宽(内容内缩量),默认 12

    void setResizeMargin(int margin); // 边缘拉伸判定的宽度(逻辑像素)
    int resizeMargin() const;

    QSize minimumSizeHint() const override; // 内容最小尺寸 + 阴影环

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    FramelessHandler *m_handler = nullptr;
    FramelessShadow *m_shadow = nullptr;
};
