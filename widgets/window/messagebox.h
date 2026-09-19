#pragma once

#include "framelessdialog.h"

class QLabel;

// 风格化消息框:FramelessDialog + TitleBar + 主题按钮组成的提示/确认弹窗,
// 图标与配色取自 Theme。用法对齐 QMessageBox:
//   MessageBox::information(this, "提示", "已保存");
//   if(MessageBox::question(this, "删除", "确定删除这首歌吗?")) { ... }
class MessageBox : public FramelessDialog
{
    Q_OBJECT
public:
    enum class Icon
    {
        None,
        Information,  // 主色 i
        Warning,      // 警告色 !
        Critical,     // 危险色 ✕
        Question      // 主色 ?
    };

    explicit MessageBox(const QString &title = {}, const QString &text = {},
                        Icon icon = Icon::Information, QWidget *parent = nullptr);

    void setText(const QString &text);
    QString text() const;

    // 模态便捷入口(question 返回是否点了"确定";Esc/关闭视为取消)
    static void information(QWidget *parent, const QString &title, const QString &text);
    static void warning(QWidget *parent, const QString &title, const QString &text);
    static void critical(QWidget *parent, const QString &title, const QString &text);
    static bool question(QWidget *parent, const QString &title, const QString &text);

private:
    QLabel *m_textLabel = nullptr;
};
