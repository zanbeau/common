#include "titlebarpage.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "framelesswidget.h"
#include "pushbutton.h"
#include "titlebar.h"

TitleBarPage::TitleBarPage(QWidget *parent)
    : QWidget(parent)
{
    QLabel *intro = new QLabel(QStringLiteral(
        "TitleBar:无边框窗口的标题栏(标题 + 最小化/最大化/关闭),主题化绘制。\n"
        "标题栏本体对鼠标透明——按下即拖动窗口、双击即最大化(走 FramelessHandler),\n"
        "只有右上角三个按钮自己响应点击。"));
    intro->setTextInteractionFlags(Qt::TextSelectableByMouse);

    PushButton *open = new PushButton(QStringLiteral("打开带标题栏的窗口"), PushButton::Type::Primary);
    connect(open, &PushButton::clicked, this, []() {
        auto *window = new FramelessWidget;
        window->setAttribute(Qt::WA_DeleteOnClose);
        window->setWindowTitle(QStringLiteral("标题栏演示"));

        QVBoxLayout *layout = new QVBoxLayout(window);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        TitleBar *bar = new TitleBar(window, QStringLiteral("我是一个标题栏"));
        layout->addWidget(bar);

        QLabel *hint = new QLabel(QStringLiteral(
            "拖动标题栏移动窗口;双击标题栏最大化/还原;\n"
            "靠近窗口边缘可八方向拉伸。"));
        hint->setAlignment(Qt::AlignCenter);
        layout->addWidget(hint, 1);

        QPushButton *close = new QPushButton(QStringLiteral("关闭"));
        QObject::connect(close, &QPushButton::clicked, window, &QWidget::close);
        layout->addWidget(close, 0, Qt::AlignHCenter);

        window->resize(420, 260);
        window->show();
    });

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->addWidget(intro);
    layout->addStretch();
    layout->addWidget(open, 0, Qt::AlignHCenter);
    layout->addStretch();
}
