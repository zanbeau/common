#include "toggleswitchpage.h"

#include <QLabel>
#include <QVBoxLayout>

#include "toggleswitch.h"

ToggleSwitchPage::ToggleSwitchPage(QWidget *parent)
    : QWidget(parent)
{
    QLabel *intro = new QLabel(QStringLiteral(
        "ToggleSwitch:开关,点击或空格切换,滑块位移带动效。\n"
        "作为对照,右侧是禁用态。"));
    intro->setTextInteractionFlags(Qt::TextSelectableByMouse);

    ToggleSwitch *toggle = new ToggleSwitch;
    toggle->setChecked(true);
    QLabel *state = new QLabel(QStringLiteral("状态:开"));
    connect(toggle, &ToggleSwitch::toggled, this, [state](bool checked) {
        state->setText(checked ? QStringLiteral("状态:开") : QStringLiteral("状态:关"));
    });

    ToggleSwitch *disabled = new ToggleSwitch;
    disabled->setChecked(true);
    disabled->setEnabled(false);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->addWidget(intro);
    layout->addStretch();
    layout->addWidget(toggle, 0, Qt::AlignCenter);
    layout->addWidget(state, 0, Qt::AlignHCenter);
    layout->addSpacing(16);
    layout->addWidget(disabled, 0, Qt::AlignCenter);
    layout->addStretch();
}
