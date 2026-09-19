#include "themepage.h"

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

#include "pushbutton.h"
#include "theme.h"

namespace {

const struct
{
    Theme::Role role;
    const char *name;
} kRoles[] = {
    {Theme::Role::Primary, "Primary"},
    {Theme::Role::PrimaryHover, "PrimaryHover"},
    {Theme::Role::PrimaryPressed, "PrimaryPressed"},
    {Theme::Role::Success, "Success"},
    {Theme::Role::Warning, "Warning"},
    {Theme::Role::Danger, "Danger"},
    {Theme::Role::Background, "Background"},
    {Theme::Role::Surface, "Surface"},
    {Theme::Role::SurfaceVariant, "SurfaceVariant"},
    {Theme::Role::Border, "Border"},
    {Theme::Role::BorderStrong, "BorderStrong"},
    {Theme::Role::Text, "Text"},
    {Theme::Role::TextSecondary, "TextSecondary"},
    {Theme::Role::TextTertiary, "TextTertiary"},
    {Theme::Role::TextDisabled, "TextDisabled"},
    {Theme::Role::TextOnPrimary, "TextOnPrimary"},
};

} // namespace

ThemePage::ThemePage(QWidget *parent)
    : QWidget(parent)
{
    QLabel *intro = new QLabel(QStringLiteral(
        "Theme:亮/暗双主题的设计令牌引擎。\n"
        "色板参考 Arco,圆角取 Fluent 桌面惯例;apply() 后基础件整体换肤。\n"
        "切到暗色看看整个控件浏览器的变化(左侧列表、按钮、输入框都在内)。"));
    intro->setTextInteractionFlags(Qt::TextSelectableByMouse);

    PushButton *lightButton = new PushButton(QStringLiteral("亮色"), PushButton::Type::Primary);
    PushButton *darkButton = new PushButton(QStringLiteral("暗色"));
    connect(lightButton, &PushButton::clicked, this, []() {
        Theme::instance()->setMode(Theme::Mode::Light);
    });
    connect(darkButton, &PushButton::clicked, this, []() {
        Theme::instance()->setMode(Theme::Mode::Dark);
    });

    // 语义色卡:主题切换时重建
    QGridLayout *swatches = new QGridLayout;
    swatches->setSpacing(8);
    const auto rebuild = [swatches]() {
        while(QLayoutItem *item = swatches->takeAt(0))
        {
            delete item->widget();
            delete item;
        }
        int index = 0;
        for(const auto &entry : kRoles)
        {
            QFrame *card = new QFrame;
            card->setFixedSize(96, 48);
            const QColor color = Theme::instance()->color(entry.role);
            card->setStyleSheet(QStringLiteral("background:%1;border-radius:4px;")
                                    .arg(color.name()));
            // 按底色亮度选字色:浅底用深字,饱和/深底用白字,避免主题切换后文字不可读
            const qreal luma = 0.299 * color.redF() + 0.587 * color.greenF()
                             + 0.114 * color.blueF();
            const QColor nameColor = luma > 0.6 ? QColor(0x1D, 0x21, 0x29)
                                                : QColor(0xFF, 0xFF, 0xFF);
            QLabel *name = new QLabel(QString::fromLatin1(entry.name), card);
            name->setGeometry(card->rect()); // 铺满卡片,配合 AlignCenter 居中
            name->setAlignment(Qt::AlignCenter);
            name->setStyleSheet(QStringLiteral(
                "background:transparent;color:%1;font-size:11px;")
                .arg(nameColor.name()));
            swatches->addWidget(card, index / 4, index % 4);
            ++index;
        }
    };
    rebuild();
    connect(Theme::instance(), &Theme::modeChanged, this, [rebuild]() { rebuild(); });

    QLabel *tokenInfo = new QLabel(QStringLiteral(
        "尺寸令牌:半径 SM/MD/LG = 4/6/8px,间距阶梯 4/8/12/16/24,\n"
        "字号 12/13/14/16/20,控件高度 28/32/36,动效 150/250/400ms。"));
    tokenInfo->setTextInteractionFlags(Qt::TextSelectableByMouse);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->addWidget(intro);
    layout->addSpacing(8);
    QHBoxLayout *modeRow = new QHBoxLayout;
    modeRow->addWidget(lightButton);
    modeRow->addWidget(darkButton);
    modeRow->addStretch();
    layout->addLayout(modeRow);
    layout->addSpacing(8);
    layout->addLayout(swatches);
    layout->addStretch();
    layout->addWidget(tokenInfo);
}
