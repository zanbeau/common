#pragma once

#include <QDialog>

class FramelessHandler;

// 无边框对话框基类:与 FramelessWidget 同一套拖动/边缘拉伸/双击最大化行为
// (FramelessHandler),供"关于/确认/设置"这类弹窗使用;
// 标题栏内容不在基类里,由使用方自行摆放
class FramelessDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FramelessDialog(QWidget *parent = nullptr);

    void setResizeMargin(int margin); // 边缘拉伸判定的宽度(逻辑像素)
    int resizeMargin() const;

private:
    FramelessHandler *m_handler = nullptr;
};
