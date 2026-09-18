#include "animationstackedpage.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "animationstackedwidget.h"

AnimationStackedPage::AnimationStackedPage(QWidget *parent)
    : QWidget(parent)
{
    QLabel *intro = new QLabel(QStringLiteral(
        "AnimationStackedWidget:页面切换时新页从前进方向滑入、旧页滑出,\n"
        "横向/纵向可选,动画期间的切换请求会被忽略。适合主界面分页切换。"));
    intro->setTextInteractionFlags(Qt::TextSelectableByMouse);

    m_stack = new AnimationStackedWidget;
    m_stack->setFixedHeight(160);
    for(int i = 1; i <= 3; ++i)
    {
        QLabel *page = new QLabel(QStringLiteral("第 %1 页").arg(i));
        page->setAlignment(Qt::AlignCenter);
        page->setStyleSheet(QStringLiteral("background:%1;").arg(
            i == 1 ? QStringLiteral("#dcebfd") : (i == 2 ? QStringLiteral("#dcf5e6")
                                                          : QStringLiteral("#fbe9dc"))));
        m_stack->addWidget(page);
    }

    m_next = new QPushButton(QStringLiteral("下一页(循环)"));
    connect(m_next, &QPushButton::clicked, this, &AnimationStackedPage::switchPage);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->addWidget(intro);
    layout->addStretch();
    layout->addWidget(m_stack);
    layout->addWidget(m_next, 0, Qt::AlignCenter);
    layout->addStretch();
}

void AnimationStackedPage::switchPage()
{
    m_stack->setCurrentIndexAnimated((m_stack->currentIndex() + 1) % m_stack->count());
}
