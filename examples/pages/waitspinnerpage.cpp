#include "waitspinnerpage.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "waitspinner.h"

WaitSpinnerPage::WaitSpinnerPage(QWidget *parent)
    : QWidget(parent)
{
    QLabel *intro = new QLabel(QStringLiteral(
        "WaitSpinner:不定进度的转圈指示,周期/颜色/线宽可调,停止时不绘制。\n"
        "用于加载中、后台扫描等没有确定进度的等待场景。"));
    intro->setTextInteractionFlags(Qt::TextSelectableByMouse);

    m_spinner = new WaitSpinner;
    m_spinner->setFixedSize(48, 48);
    m_spinner->start();

    m_toggle = new QPushButton(QStringLiteral("停止"));
    connect(m_toggle, &QPushButton::clicked, this, [this]() {
        if(m_spinner->isSpinning())
        {
            m_spinner->stop();
            m_toggle->setText(QStringLiteral("启动"));
        }
        else
        {
            m_spinner->start();
            m_toggle->setText(QStringLiteral("停止"));
        }
    });

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->addWidget(intro);
    layout->addStretch();
    layout->addWidget(m_spinner, 0, Qt::AlignCenter);
    layout->addWidget(m_toggle, 0, Qt::AlignCenter);
    layout->addStretch();
}
