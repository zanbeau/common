#include "framelesspage.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "framelessdemowindow.h"

FramelessPage::FramelessPage(QWidget *parent)
    : QWidget(parent)
{
    QLabel *intro = new QLabel(QStringLiteral(
        "FramelessWidget:无边框窗口基类,期待作为顶层窗口使用\n\n"
        "  · 按住客户区可拖动窗口\n"
        "  · 靠近窗口边缘时按住可拉伸(八个方向)\n"
        "  · 双击客户区最大化/还原\n"
        "  · 拖动/拉伸优先交给窗口系统处理(原生贴边手感、Wayland 兼容),\n"
        "    窗口系统不支持时回退为手动实现"));
    intro->setTextInteractionFlags(Qt::TextSelectableByMouse);

    QPushButton *button = new QPushButton(QStringLiteral("打开无边框窗口示例"));
    connect(button, &QPushButton::clicked, this, [this]() {
        FramelessDemoWindow *window = new FramelessDemoWindow(this);
        window->setAttribute(Qt::WA_DeleteOnClose);
        window->show();
    });

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->addWidget(intro);
    layout->addStretch();
    layout->addWidget(button, 0, Qt::AlignCenter);
    layout->addStretch();
}
