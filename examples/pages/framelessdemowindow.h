#pragma once

#include "framelesswidget.h"

// 无边框窗口演示窗口:标题栏(空白处拖动/双击最大化/关闭按钮)+ 内容区
class FramelessDemoWindow : public FramelessWidget
{
    Q_OBJECT
public:
    explicit FramelessDemoWindow(QWidget *parent = nullptr);
};
