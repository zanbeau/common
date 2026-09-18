#include "marqueepage.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "marqueelabel.h"

MarqueePage::MarqueePage(QWidget *parent)
    : QWidget(parent)
{
    QLabel *intro = new QLabel(QStringLiteral(
        "MarqueeLabel:文字超宽时自动横向滚动,hover 暂停,不超宽就是普通 QLabel。\n"
        "适合歌名、通知等长文本的展示。"));
    intro->setTextInteractionFlags(Qt::TextSelectableByMouse);

    m_label = new MarqueeLabel;
    m_label->setText(QStringLiteral("这是一段很长很长很长很长很长的滚动文本 —— 演示 MarqueeLabel 的循环滚动效果"));
    m_label->setFixedWidth(320);
    m_label->setStyleSheet(QStringLiteral("border:1px solid #d0d0d0; padding:4px;"));

    m_toggle = new QPushButton(QStringLiteral("切换滚动方向"));
    connect(m_toggle, &QPushButton::clicked, this, [this]() {
        m_label->setDirection(m_label->direction() == MarqueeLabel::ScrollLeft
                                  ? MarqueeLabel::ScrollRight
                                  : MarqueeLabel::ScrollLeft);
    });

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->addWidget(intro);
    layout->addStretch();
    layout->addWidget(m_label, 0, Qt::AlignCenter);
    layout->addWidget(m_toggle, 0, Qt::AlignCenter);
    layout->addStretch();
}
