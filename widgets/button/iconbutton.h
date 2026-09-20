#pragma once

#include <QAbstractButton>

class QColor;
class QPainter;
class QRectF;

// 图标按钮:纯图标无文字,字形全部 QPainter 绘制,随主题换色
//  - 悬浮/按下画淡色底,checkable 选中态为主色淡底 + 主色图标
//  - setGlyph() 运行时换图标(播放⇄暂停、音量⇄静音这类切换)
//  - paintGlyph() 公开静态:其他自绘控件(菜单项、标题栏等)可复用同一套字形,
//    保证整个应用的图标风格一致
class IconButton : public QAbstractButton
{
    Q_OBJECT
public:
    enum class Glyph
    {
        Play, Pause, Stop,
        SkipPrevious, SkipNext,
        Repeat, RepeatOne, Shuffle,
        VolumeLow, VolumeHigh, VolumeMute,
        Plus, Minus, Close, Check,
        ChevronLeft, ChevronRight, ChevronUp, ChevronDown,
        Ellipsis, Heart
    };
    Q_ENUM(Glyph)

    explicit IconButton(Glyph glyph = Glyph::Play, QWidget *parent = nullptr);

    Glyph glyph() const;
    void setGlyph(Glyph glyph);        // 运行时换图标,立即重绘

    int iconSize() const;              // 图标边长(px)
    void setIconSize(int size);

    QSize sizeHint() const override;

    // 在 rect(应为正方形)内以 color 绘制 glyph。
    // 字形设计在 16x16 坐标系里,按 rect 等比缩放,线宽随之缩放
    static void paintGlyph(QPainter *painter, Glyph glyph,
                           const QRectF &rect, const QColor &color);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Glyph m_glyph;
    int m_iconSize = 16;
};
