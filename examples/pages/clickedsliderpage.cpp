#include "clickedsliderpage.h"

#include <QLabel>
#include <QVBoxLayout>

#include "clickedslider.h"

ClickedSliderPage::ClickedSliderPage(QWidget *parent)
    : QWidget(parent)
{
    QLabel *intro = new QLabel(QStringLiteral(
        "ClickedSlider:点击轨道任意位置手柄直接跳过去,按住还能继续拖动。\n"
        "做音量条/播放进度条不用再点好几下才够得着目标位置。"));
    intro->setTextInteractionFlags(Qt::TextSelectableByMouse);

    m_slider = new ClickedSlider(Qt::Horizontal);
    m_slider->setRange(0, 100);
    m_slider->setValue(30);

    m_value = new QLabel(QStringLiteral("当前值:30"));
    connect(m_slider, &QSlider::valueChanged, this, [this](int value) {
        m_value->setText(QStringLiteral("当前值:%1").arg(value));
    });

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->addWidget(intro);
    layout->addStretch();
    layout->addWidget(m_slider);
    layout->addWidget(m_value, 0, Qt::AlignCenter);
    layout->addStretch();
}
