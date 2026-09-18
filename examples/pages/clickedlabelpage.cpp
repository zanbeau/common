#include "clickedlabelpage.h"

#include <QLabel>
#include <QVariant>
#include <QVBoxLayout>

#include "clickedlabel.h"

ClickedLabelPage::ClickedLabelPage(QWidget *parent)
    : QWidget(parent)
{
    QLabel *intro = new QLabel(QStringLiteral(
        "ClickedLabel:可点击的 QLabel,左键在标签内按下并释放时发出 clicked() 信号,\n"
        "按下事件会被拦截不再向上传播,适合放进无边框窗口的标题栏做自定义按钮。"));
    intro->setTextInteractionFlags(Qt::TextSelectableByMouse);

    ClickedLabel *link = new ClickedLabel(QStringLiteral("点我(0)"));
    link->setCursor(Qt::PointingHandCursor);
    link->setStyleSheet(QStringLiteral("color:#0a66c2;"));

    QLabel *count = new QLabel(QStringLiteral("已点击 0 次"));
    count->setProperty("count", 0);
    connect(link, &ClickedLabel::clicked, count, [link, count]() {
        const int n = count->property("count").toInt() + 1;
        count->setProperty("count", n);
        link->setText(QStringLiteral("点我(%1)").arg(n));
        count->setText(QStringLiteral("已点击 %1 次").arg(n));
    });

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->addWidget(intro);
    layout->addStretch();
    layout->addWidget(link, 0, Qt::AlignCenter);
    layout->addWidget(count, 0, Qt::AlignCenter);
    layout->addStretch();
}
