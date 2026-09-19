#include "sidenavpage.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QStackedWidget>

#include "sidenav.h"
#include "theme.h"

namespace {

// 页面占位:主题化底色 + 居中标题
QWidget *makePlaceholder(const QString &title)
{
    auto *page = new QLabel(title);
    page->setAlignment(Qt::AlignCenter);
    page->setStyleSheet(QStringLiteral("background:%1;color:%2;font-size:20px;")
                            .arg(Theme::instance()->color(Theme::Role::Background).name())
                            .arg(Theme::instance()->color(Theme::Role::Text).name()));
    return page;
}

}

SideNavPage::SideNavPage(QWidget *parent)
    : QWidget(parent)
{
    QLabel *intro = new QLabel(QStringLiteral(
        "SideNav:侧边导航(左列表 + 右内容),GalleryWindow 同款布局的库化版本。\n"
        "点击 / 上下键切换,选中项淡主色底 + 左侧指示条。"));
    intro->setTextInteractionFlags(Qt::TextSelectableByMouse);

    SideNav *nav = new SideNav;
    QStackedWidget *stack = new QStackedWidget;
    const char *items[] = {"发现音乐", "推荐歌单", "我的喜欢", "最近播放", "设置"};
    for(const char *item : items)
    {
        nav->addItem(QString::fromLatin1(item));
        stack->addWidget(makePlaceholder(QString::fromLatin1(item)));
    }
    connect(nav, &SideNav::currentChanged, stack, &QStackedWidget::setCurrentIndex);
    connect(stack, &QStackedWidget::currentChanged, this, [stack](int index) {
        // 内容页反向同步回导航(演示双向联动)
        Q_UNUSED(index)
        stack->update();
    });

    QHBoxLayout *body = new QHBoxLayout;
    body->setSpacing(8);
    body->addWidget(nav);
    body->addWidget(stack, 1);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->addWidget(intro);
    layout->addLayout(body, 1);
}
