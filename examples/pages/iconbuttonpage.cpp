#include "iconbuttonpage.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

#include "iconbutton.h"
#include "toastlabel.h"

IconButtonPage::IconButtonPage(QWidget *parent)
    : QWidget(parent)
{
    QLabel *intro = new QLabel(QStringLiteral(
        "IconButton:纯图标按钮,字形全部 QPainter 绘制,随主题换色。\n"
        "悬浮/按下淡色底;可 checkable,选中态主色淡底 + 主色图标;\n"
        "setGlyph() 运行时换图标(下方播放条的 播放⇄暂停、音量⇄静音)。"));
    intro->setTextInteractionFlags(Qt::TextSelectableByMouse);

    // 全部字形一览:可点选查看选中态
    struct NamedGlyph { IconButton::Glyph glyph; const char *name; };
    const NamedGlyph glyphs[] = {
        {IconButton::Glyph::Play, "Play"},
        {IconButton::Glyph::Pause, "Pause"},
        {IconButton::Glyph::Stop, "Stop"},
        {IconButton::Glyph::SkipPrevious, "SkipPrevious"},
        {IconButton::Glyph::SkipNext, "SkipNext"},
        {IconButton::Glyph::Repeat, "Repeat"},
        {IconButton::Glyph::RepeatOne, "RepeatOne"},
        {IconButton::Glyph::Shuffle, "Shuffle"},
        {IconButton::Glyph::VolumeLow, "VolumeLow"},
        {IconButton::Glyph::VolumeHigh, "VolumeHigh"},
        {IconButton::Glyph::VolumeMute, "VolumeMute"},
        {IconButton::Glyph::Plus, "Plus"},
        {IconButton::Glyph::Minus, "Minus"},
        {IconButton::Glyph::Close, "Close"},
        {IconButton::Glyph::Check, "Check"},
        {IconButton::Glyph::ChevronLeft, "ChevronLeft"},
        {IconButton::Glyph::ChevronRight, "ChevronRight"},
        {IconButton::Glyph::ChevronUp, "ChevronUp"},
        {IconButton::Glyph::ChevronDown, "ChevronDown"},
        {IconButton::Glyph::Ellipsis, "Ellipsis"},
        {IconButton::Glyph::Heart, "Heart"},
    };
    QGridLayout *grid = new QGridLayout;
    grid->setSpacing(8);
    int row = 0, column = 0;
    for(const NamedGlyph &named : glyphs)
    {
        IconButton *button = new IconButton(named.glyph);
        button->setCheckable(true);
        button->setToolTip(QString::fromLatin1(named.name));
        grid->addWidget(button, row, column);
        if(++column == 7)
        {
            column = 0;
            ++row;
        }
    }

    // 迷你播放条:图标运行时切换的典型用法
    QLabel *playerCaption = new QLabel(QStringLiteral("播放条用法:"));
    IconButton *prev = new IconButton(IconButton::Glyph::SkipPrevious);
    IconButton *play = new IconButton(IconButton::Glyph::Play);
    IconButton *next = new IconButton(IconButton::Glyph::SkipNext);
    play->setIconSize(24);
    prev->setToolTip(QStringLiteral("上一首"));
    play->setToolTip(QStringLiteral("播放/暂停"));
    next->setToolTip(QStringLiteral("下一首"));
    connect(play, &IconButton::clicked, this, [this, play]() {
        play->setGlyph(play->glyph() == IconButton::Glyph::Play
                           ? IconButton::Glyph::Pause
                           : IconButton::Glyph::Play);
        ToastLabel::showText(play->glyph() == IconButton::Glyph::Pause
                                 ? QStringLiteral("播放中")
                                 : QStringLiteral("已暂停"),
                             window());
    });

    IconButton *mute = new IconButton(IconButton::Glyph::VolumeHigh);
    mute->setCheckable(true);
    mute->setToolTip(QStringLiteral("静音"));
    connect(mute, &IconButton::toggled, this, [mute](bool muted) {
        mute->setGlyph(muted ? IconButton::Glyph::VolumeMute
                             : IconButton::Glyph::VolumeHigh);
    });

    QHBoxLayout *player = new QHBoxLayout;
    player->setSpacing(8);
    player->addWidget(playerCaption);
    player->addWidget(prev);
    player->addWidget(play);
    player->addWidget(next);
    player->addSpacing(24);
    player->addWidget(mute);
    player->addStretch();

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(24);
    layout->addWidget(intro);
    layout->addLayout(grid);
    layout->addStretch();
    layout->addLayout(player);
}
