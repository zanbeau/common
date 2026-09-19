#include "notifywindowpage.h"

#include <QLabel>
#include <QVBoxLayout>

#include "notifywindow.h"
#include "pushbutton.h"

NotifyWindowPage::NotifyWindowPage(QWidget *parent)
    : QWidget(parent)
{
    QLabel *intro = new QLabel(QStringLiteral(
        "NotifyWindow:主屏右下角的桌面通知(切歌提示用),\n"
        "到点自动淡出;多条通知自下而上堆叠,点击通知即关闭。"));
    intro->setTextInteractionFlags(Qt::TextSelectableByMouse);

    PushButton *single = new PushButton(QStringLiteral("弹一条通知"), PushButton::Type::Primary);
    connect(single, &PushButton::clicked, this, []() {
        NotifyWindow::showMessage(QStringLiteral("正在播放"),
                                  QStringLiteral("晴天 - 周杰伦"));
    });

    PushButton *multi = new PushButton(QStringLiteral("连弹三条"));
    int index = 0;
    connect(multi, &PushButton::clicked, this, [index]() mutable {
        static const char *songs[] = {"七里香 - 周杰伦", "稻香 - 周杰伦", "夜曲 - 周杰伦"};
        for(int i = 0; i < 3; ++i)
        {
            NotifyWindow::showMessage(QStringLiteral("正在播放"),
                                      QString::fromLatin1(songs[(index + i) % 3]));
        }
        index += 3;
    });

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->addWidget(intro);
    layout->addStretch();
    layout->addWidget(single, 0, Qt::AlignHCenter);
    layout->addWidget(multi, 0, Qt::AlignHCenter);
    layout->addStretch();
}
