#include "pushbuttonpage.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

#include "pushbutton.h"
#include "toastlabel.h"

PushButtonPage::PushButtonPage(QWidget *parent)
    : QWidget(parent)
{
    QLabel *intro = new QLabel(QStringLiteral(
        "PushButton:四种形态(Default/Primary/Danger/Text),自绘实现,\n"
        "颜色取自 Theme 令牌,亮暗主题与悬浮/按下态自动联动。"));
    intro->setTextInteractionFlags(Qt::TextSelectableByMouse);

    QHBoxLayout *row = new QHBoxLayout;
    PushButton *def = new PushButton(QStringLiteral("默认"));
    PushButton *primary = new PushButton(QStringLiteral("主要"), PushButton::Type::Primary);
    PushButton *danger = new PushButton(QStringLiteral("危险"), PushButton::Type::Danger);
    PushButton *text = new PushButton(QStringLiteral("文字按钮"), PushButton::Type::Text);
    PushButton *disabled = new PushButton(QStringLiteral("禁用"), PushButton::Type::Primary);
    disabled->setEnabled(false);
    row->addWidget(def);
    row->addWidget(primary);
    row->addWidget(danger);
    row->addWidget(text);
    row->addWidget(disabled);
    row->addStretch();

    for(PushButton *button : {def, primary, danger, text})
    {
        connect(button, &PushButton::clicked, this, [this, button]() {
            ToastLabel::showText(button->text() + QStringLiteral(" 被点击了"), window());
        });
    }

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->addWidget(intro);
    layout->addStretch();
    layout->addLayout(row);
    layout->addStretch();
}
