#include "rotatelabelpage.h"

#include <QCheckBox>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>

#include "pushbutton.h"
#include "rotatelabel.h"

namespace {

// 模拟唱片封面:中心圆孔 + 环形纹理
QPixmap makeDisc()
{
    QPixmap pixmap(320, 320);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(pixmap.rect(), QColor(0x2E, 0x2E, 0x30));
    painter.setPen(QPen(QColor(0x52, 0x52, 0x57), 2));
    for(int r = 40; r < 160; r += 16)
    {
        painter.drawEllipse(160, 160, r * 2, r * 2);
    }
    painter.setBrush(QColor(0x16, 0x5D, 0xFF));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(144, 144, 32, 32);
    return pixmap;
}

} // namespace

RotateLabelPage::RotateLabelPage(QWidget *parent)
    : QWidget(parent)
{
    QLabel *intro = new QLabel(QStringLiteral(
        "RotateLabel:旋转封面(唱片效果),可选圆形裁剪,\n"
        "一圈的时长可调,默认静止、setRunning(true) 开始转。"));
    intro->setTextInteractionFlags(Qt::TextSelectableByMouse);

    RotateLabel *disc = new RotateLabel;
    disc->setFixedSize(200, 200);
    disc->setPixmap(makeDisc());
    disc->setCircular(true);

    QPushButton *toggle = new PushButton(QStringLiteral("开始旋转"), PushButton::Type::Primary);
    connect(toggle, &PushButton::clicked, this, [disc, toggle]() {
        disc->setRunning(!disc->isRunning());
        toggle->setText(disc->isRunning() ? QStringLiteral("暂停")
                                          : QStringLiteral("开始旋转"));
    });

    QCheckBox *circular = new QCheckBox(QStringLiteral("圆形裁剪"));
    circular->setChecked(true);
    connect(circular, &QCheckBox::toggled, disc, &RotateLabel::setCircular);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->addWidget(intro);
    layout->addStretch();
    layout->addWidget(disc, 0, Qt::AlignCenter);
    layout->addSpacing(12);
    layout->addWidget(toggle, 0, Qt::AlignHCenter);
    layout->addWidget(circular, 0, Qt::AlignHCenter);
    layout->addStretch();
}
