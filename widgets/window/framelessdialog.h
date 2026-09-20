#pragma once

#include <QDialog>

class FramelessHandler;
class FramelessShadow;
class QVBoxLayout;

// 无边框对话框基类:与 FramelessWidget 同一套拖动/边缘拉伸/双击最大化行为
// (FramelessHandler)与阴影/圆角外观(FramelessShadow),供"关于/确认/设置"
// 这类弹窗使用;业务布局放在 contentLayout() 上,标题栏由使用方自行摆放
class FramelessDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FramelessDialog(QWidget *parent = nullptr);

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
