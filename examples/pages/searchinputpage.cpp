#include "searchinputpage.h"

#include <QLabel>
#include <QVBoxLayout>

#include "searchinput.h"

SearchInputPage::SearchInputPage(QWidget *parent)
    : QWidget(parent)
{
    QLabel *intro = new QLabel(QStringLiteral(
        "SearchInput:前置放大镜图标 + 内置清除按钮的搜索框,\n"
        "回车发出 searchRequested(text);图标随亮暗主题自动换色。"));
    intro->setTextInteractionFlags(Qt::TextSelectableByMouse);

    SearchInput *input = new SearchInput(QStringLiteral("搜索歌曲 / 歌手"));
    input->setFixedWidth(280);

    QLabel *echo = new QLabel(QStringLiteral("回车提交搜索词"));
    echo->setAlignment(Qt::AlignHCenter);
    connect(input, &SearchInput::searchRequested, this, [echo](const QString &text) {
        echo->setText(QStringLiteral("搜索:%1").arg(text));
    });

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->addWidget(intro);
    layout->addStretch();
    layout->addWidget(input, 0, Qt::AlignHCenter);
    layout->addSpacing(8);
    layout->addWidget(echo);
    layout->addStretch();
}
