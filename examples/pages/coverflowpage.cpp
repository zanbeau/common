#include "coverflowpage.h"

#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QRandomGenerator>
#include <QVBoxLayout>

#include "coverflow.h"
#include "pushbutton.h"

namespace {

// 演示封面:双色渐变 + 序号
QPixmap makeCover(int index)
{
    static const QColor tops[] = {
        QColor(0x16, 0x5D, 0xFF), QColor(0x00, 0xB4, 0x2A), QColor(0xFF, 0x7D, 0x00),
        QColor(0xF5, 0x3F, 0x3F), QColor(0x7B, 0x2F, 0xFF), QColor(0x00, 0xA8, 0xA8),
        QColor(0xD9, 0x30, 0x8C)};
    static const QColor bottoms[] = {
        QColor(0x0E, 0x42, 0xD2), QColor(0x00, 0x6A, 0x19), QColor(0xD9, 0x60, 0x00),
        QColor(0xC1, 0x24, 0x24), QColor(0x4E, 0x1D, 0xCC), QColor(0x00, 0x6F, 0x6F),
        QColor(0xA5, 0x1E, 0x6B)};

    QPixmap pixmap(320, 320);
    QPainter painter(&pixmap);
    QLinearGradient gradient(0, 0, 0, 320);
    gradient.setColorAt(0.0, tops[index % 7]);
    gradient.setColorAt(1.0, bottoms[index % 7]);
    painter.fillRect(pixmap.rect(), gradient);
    painter.setPen(Qt::white);
    QFont font = painter.font();
    font.setPixelSize(120);
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(pixmap.rect(), Qt::AlignCenter, QString::number(index + 1));
    return pixmap;
}

}

CoverFlowPage::CoverFlowPage(QWidget *parent)
    : QWidget(parent)
{
    QLabel *intro = new QLabel(QStringLiteral(
        "CoverFlow:封面流——中间大、两侧渐小渐淡、带倒影。\n"
        "滚轮 / 左右键切换,按住拖动翻页;松手后滑动吸附到最近的封面。"));
    intro->setTextInteractionFlags(Qt::TextSelectableByMouse);

    CoverFlow *flow = new CoverFlow;
    QVector<QPixmap> covers;
    for(int i = 0; i < 7; ++i)
    {
        covers.append(makeCover(i));
    }
    flow->setCovers(covers);

    QLabel *current = new QLabel(QStringLiteral("当前:1 / 7"));
    current->setAlignment(Qt::AlignHCenter);
    connect(flow, &CoverFlow::currentChanged, this, [current](int index) {
        current->setText(QStringLiteral("当前:%1 / 7").arg(index + 1));
    });

    PushButton *shuffle = new PushButton(QStringLiteral("随机跳一张"), PushButton::Type::Primary);
    connect(shuffle, &PushButton::clicked, this, [flow]() {
        flow->setCurrentIndex(QRandomGenerator::global()->bounded(7));
    });

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->addWidget(intro);
    layout->addWidget(flow, 1);
    layout->addWidget(current);
    layout->addWidget(shuffle, 0, Qt::AlignHCenter);
}
