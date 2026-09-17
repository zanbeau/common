// widgets 示例:无边框窗口 + 标题栏关闭按钮(ClickedLabel)+ Toast
#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "clickedlabel.h"
#include "framelesswidget.h"
#include "toastlabel.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    FramelessWidget window;
    window.setObjectName("window");
    window.setStyleSheet(QStringLiteral(
        "#window{background:#f4f4f4;}"
        "#titleBar{background:#2d2d30;}"
        "#titleLabel{color:#ffffff;font-size:13px;}"
        "#closeLabel{color:#ffffff;font-size:14px;border-radius:4px;}"
        "#closeLabel:hover{background:#e81123;}"));
    window.resize(560, 380);
    window.setMinimumSize(360, 240);

    QVBoxLayout *layout = new QVBoxLayout(&window);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // 标题栏:空白处可拖动/双击最大化,右侧 ClickedLabel 充当关闭按钮
    QWidget *titleBar = new QWidget;
    titleBar->setObjectName("titleBar");
    titleBar->setFixedHeight(36);
    QHBoxLayout *titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(12, 0, 12, 0);
    QLabel *titleLabel = new QLabel(QStringLiteral("common 示例"));
    titleLabel->setObjectName("titleLabel");
    ClickedLabel *closeLabel = new ClickedLabel(QStringLiteral("✕"));
    closeLabel->setObjectName("closeLabel");
    closeLabel->setFixedSize(32, 24);
    closeLabel->setAlignment(Qt::AlignCenter);
    QObject::connect(closeLabel, &ClickedLabel::clicked, &window, &QWidget::close);
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
    QObject::connect(toastButton, &QPushButton::clicked, &window, [&window]() {
        ToastLabel::showText(QStringLiteral("这是一条 Toast 提示"), &window);
    });
    bodyLayout->addWidget(hintLabel);
    bodyLayout->addStretch();
    bodyLayout->addWidget(toastButton, 0, Qt::AlignCenter);
    bodyLayout->addStretch();

    layout->addWidget(titleBar);
    layout->addWidget(body, 1);

    window.show();
    return app.exec();
}
