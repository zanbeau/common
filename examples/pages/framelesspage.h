#pragma once

#include <QWidget>

// FramelessWidget 演示页:能力说明 + 打开示例窗口
class FramelessPage : public QWidget
{
    Q_OBJECT
public:
    explicit FramelessPage(QWidget *parent = nullptr);
};
