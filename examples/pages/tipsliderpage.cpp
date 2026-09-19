#include "tipsliderpage.h"

#include <QLabel>
#include <QVBoxLayout>

#include "duration.h"
#include "tipslider.h"

TipSliderPage::TipSliderPage(QWidget *parent)
    : QWidget(parent)
{
    QLabel *intro = new QLabel(QStringLiteral(
        "TipSlider:进度条,鼠标悬停/拖动时浮出时间气泡(默认 mm:ss)。\n"
        "range 按毫秒设置(0..总时长),点击任意位置手柄直接跳过去。"));
    intro->setTextInteractionFlags(Qt::TextSelectableByMouse);

    TipSlider *slider = new TipSlider(Qt::Horizontal);
    slider->setRange(0, 210000); // 3:30
    slider->setValue(42000);

    QLabel *current = new QLabel;
    const auto syncLabel = [current](int value) {
        current->setText(QStringLiteral("当前进度:%1 / 03:30").arg(Duration::format(value)));
    };
    syncLabel(slider->value());
    connect(slider, &TipSlider::valueChanged, this, syncLabel);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->addWidget(intro);
    layout->addStretch();
    layout->addWidget(slider);
    layout->addWidget(current, 0, Qt::AlignHCenter);
    layout->addStretch();
}
