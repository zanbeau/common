#include "toastpage.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "toastlabel.h"

ToastPage::ToastPage(QWidget *parent)
    : QWidget(parent)
{
    QLabel *intro = new QLabel(QStringLiteral(
        "ToastLabel:居中于父窗口弹出的气泡提示,停留片刻后淡出并自动销毁。\n"
        "便捷用法:ToastLabel::showText(\"提示内容\", this)"));
    intro->setTextInteractionFlags(Qt::TextSelectableByMouse);

    QPushButton *defaultButton = new QPushButton(QStringLiteral("默认时长(1600ms)"));
    connect(defaultButton, &QPushButton::clicked, this, [this]() {
        ToastLabel::showText(QStringLiteral("这是一条 Toast 提示"), window());
    });

    QPushButton *longButton = new QPushButton(QStringLiteral("停留 3 秒"));
    connect(longButton, &QPushButton::clicked, this, [this]() {
        ToastLabel::showText(QStringLiteral("停留 3 秒的 Toast"), window(), 3000);
    });

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->addWidget(intro);
    layout->addStretch();
    layout->addWidget(defaultButton, 0, Qt::AlignCenter);
    layout->addWidget(longButton, 0, Qt::AlignCenter);
    layout->addStretch();
}
