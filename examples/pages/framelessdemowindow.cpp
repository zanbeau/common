#include "framelessdemowindow.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "clickedlabel.h"
#include "framelesshandler.h"
#include "toastlabel.h"

FramelessDemoWindow::FramelessDemoWindow(QWidget *parent)
    : FramelessWidget(parent)
{
    setObjectName("window");
    setStyleSheet(QStringLiteral(
        "#window{background:#f4f4f4;}"
        "#titleBar{background:#2d2d30;}"
        "#titleLabel{color:#ffffff;font-size:13px;}"
        "#closeLabel{color:#ffffff;font-size:14px;border-radius:4px;}"
        "#closeLabel:hover{background:#e81123;}"));
    resize(560, 380);
    setMinimumSize(360, 240);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // 标题栏:空白处可拖动/双击最大化,右侧 ClickedLabel 充当关闭按钮
    QWidget *titleBar = new QWidget;
    titleBar->setObjectName("titleBar");
    titleBar->setFixedHeight(36);
    QHBoxLayout *titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(12, 0, 12, 0);
    QLabel *titleLabel = new QLabel(QStringLiteral("无边框窗口示例"));
    titleLabel->setObjectName("titleLabel");
    ClickedLabel *closeLabel = new ClickedLabel(QStringLiteral("✕"));
    closeLabel->setObjectName("closeLabel");
    closeLabel->setFixedSize(32, 24);
    closeLabel->setAlignment(Qt::AlignCenter);
    connect(closeLabel, &ClickedLabel::clicked, this, &QWidget::close);
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();
    titleLayout->addWidget(closeLabel);

    // 内容区
    QWidget *body = new QWidget;
    QVBoxLayout *bodyLayout = new QVBoxLayout(body);
    bodyLayout->setContentsMargins(24, 24, 24, 24);
    QLabel *hintLabel = new QLabel(QStringLiteral(
        "拖动标题栏移动窗口\n移近窗口边缘后拖动可拉伸\n双击标题栏最大化/还原"));
    hintLabel->setAlignment(Qt::AlignCenter);
    hintLabel->setStyleSheet(QStringLiteral("color:#555555;font-size:13px;"));
    QPushButton *toastButton = new QPushButton(QStringLiteral("弹出 Toast"));
    connect(toastButton, &QPushButton::clicked, this, [this]() {
        ToastLabel::showText(QStringLiteral("这是一条 Toast 提示"), this);
    });
    bodyLayout->addWidget(hintLabel);
    bodyLayout->addStretch();
    bodyLayout->addWidget(toastButton, 0, Qt::AlignCenter);
    bodyLayout->addStretch();

    layout->addWidget(titleBar);
    layout->addWidget(body, 1);

    // 自拼标题栏要自己挂进拖动体系(watch):拖动/双击最大化只认 watch 面板,
    // 窗口本体只保留边缘拉伸——内容区(含下方空白)按下不拖动窗口
    auto *drag = new FramelessHandler(this, this, false);
    drag->watch(titleBar);
}
