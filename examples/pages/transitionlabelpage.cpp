#include "transitionlabelpage.h"

#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>

#include "transitionlabel.h"

namespace {

// 生成演示用"封面":双色渐变 + 大写字母
QPixmap makeCover(const QColor &top, const QColor &bottom, const QString &letter)
{
    QPixmap pixmap(320, 320);
    QPainter painter(&pixmap);
    QLinearGradient gradient(0, 0, 0, 320);
    gradient.setColorAt(0.0, top);
    gradient.setColorAt(1.0, bottom);
    painter.fillRect(pixmap.rect(), gradient);
    painter.setPen(Qt::white);
    QFont font = painter.font();
    font.setPixelSize(120);
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(pixmap.rect(), Qt::AlignCenter, letter);
    return pixmap;
}

} // namespace

TransitionLabelPage::TransitionLabelPage(QWidget *parent)
    : QWidget(parent)
{
    QLabel *intro = new QLabel(QStringLiteral(
        "TransitionLabel:封面切换的交叉渐变动效(正在播放页用),\n"
        "setPixmap() 触发,首次设置直接显示。"));
    intro->setTextInteractionFlags(Qt::TextSelectableByMouse);

    TransitionLabel *cover = new TransitionLabel;
    cover->setFixedSize(220, 220);
    cover->setPixmap(makeCover(QColor(0x16, 0x5D, 0xFF), QColor(0x0E, 0x42, 0xD2),
                               QStringLiteral("A")));

    QPushButton *next = new QPushButton(QStringLiteral("换一张封面"));
    int index = 0;
    connect(next, &QPushButton::clicked, this, [cover, index]() mutable {
        index = (index + 1) % 3;
        static const QColor tops[3] = {
            QColor(0x00, 0xB4, 0x2A), QColor(0xFF, 0x7D, 0x00), QColor(0xF5, 0x3F, 0x3F)};
        static const QColor bottoms[3] = {
            QColor(0x00, 0x6A, 0x19), QColor(0xD9, 0x60, 0x00), QColor(0xC1, 0x24, 0x24)};
        const QString letters = QStringLiteral("ABC");
        cover->setPixmap(makeCover(tops[index], bottoms[index],
                                   QString(letters.at(index))));
    });

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->addWidget(intro);
    layout->addStretch();
    layout->addWidget(cover, 0, Qt::AlignCenter);
    layout->addSpacing(12);
    layout->addWidget(next, 0, Qt::AlignHCenter);
    layout->addStretch();
}
