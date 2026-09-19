#include "messageboxpage.h"

#include <QLabel>
#include <QVBoxLayout>

#include "messagebox.h"
#include "pushbutton.h"
#include "toastlabel.h"

MessageBoxPage::MessageBoxPage(QWidget *parent)
    : QWidget(parent)
{
    QLabel *intro = new QLabel(QStringLiteral(
        "MessageBox:风格化消息框(FramelessDialog + TitleBar + 主题按钮),\n"
        "用法对齐 QMessageBox:information / warning / critical / question。"));
    intro->setTextInteractionFlags(Qt::TextSelectableByMouse);

    PushButton *info = new PushButton(QStringLiteral("提示"), PushButton::Type::Primary);
    connect(info, &PushButton::clicked, this, [this]() {
        MessageBox::information(window(), QStringLiteral("提示"),
                                QStringLiteral("歌单已同步到云端。"));
    });

    PushButton *warn = new PushButton(QStringLiteral("警告"));
    connect(warn, &PushButton::clicked, this, [this]() {
        MessageBox::warning(window(), QStringLiteral("空间不足"),
                            QStringLiteral("本地磁盘剩余空间不足 500MB,下载可能失败。"));
    });

    PushButton *ask = new PushButton(QStringLiteral("确认(问句)"));
    connect(ask, &PushButton::clicked, this, [this]() {
        if(MessageBox::question(window(), QStringLiteral("删除"),
                                QStringLiteral("确定要从歌单里移除这首歌吗?")))
        {
            ToastLabel::showText(QStringLiteral("已移除"), window());
        }
    });

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->addWidget(intro);
    layout->addStretch();
    layout->addWidget(info, 0, Qt::AlignHCenter);
    layout->addWidget(warn, 0, Qt::AlignHCenter);
    layout->addWidget(ask, 0, Qt::AlignHCenter);
    layout->addStretch();
}
