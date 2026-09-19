#pragma once

#include <QColor>

// 设计令牌(design tokens):整套控件的视觉变量唯一来源
//  - 色板数值参考 Arco Design(主色 arcoblue,暗色为近似取值)
//  - 圆角/间距/动效时长取 Fluent 的桌面惯例
// 语义角色(背景/文本/主色…)不在此定义,由 Theme 按亮暗模式解析后暴露;
// 控件一律通过 Theme API 取值,保证主题切换时全局联动
namespace Tokens {

// 语义色板:亮/暗各一套,Theme 按 Mode 选择
struct Palette
{
    QColor primary;         // 主色(品牌色)
    QColor primaryHover;    // 主色悬浮
    QColor primaryPressed;  // 主色按下
    QColor success;
    QColor warning;
    QColor danger;
    QColor background;      // 窗口背景
    QColor surface;         // 控件/卡片表面
    QColor surfaceVariant;  // 次级表面(悬浮底、禁用底)
    QColor border;          // 常规描边
    QColor borderStrong;    // 强描边(悬浮态)
    QColor text;            // 主文本
    QColor textSecondary;   // 次级文本
    QColor textTertiary;    // 三级文本
    QColor textDisabled;    // 禁用文本
    QColor textOnPrimary;   // 主色上的文本(白)
    QColor inverseSurface;  // 反色表面(Toast/Tooltip 的深底)
    QColor inverseText;     // 反色表面上的文字
};

inline const Palette kLightPalette = {
    QColor(0x16, 0x5D, 0xFF),   // primary      #165DFF
    QColor(0x40, 0x80, 0xFF),   // primaryHover #4080FF
    QColor(0x0E, 0x42, 0xD2),   // primaryPressed #0E42D2
    QColor(0x00, 0xB4, 0x2A),   // success      #00B42A
    QColor(0xFF, 0x7D, 0x00),   // warning      #FF7D00
    QColor(0xF5, 0x3F, 0x3F),   // danger       #F53F3F
    QColor(0xF7, 0xF8, 0xFA),   // background   #F7F8FA
    QColor(0xFF, 0xFF, 0xFF),   // surface      #FFFFFF
    QColor(0xF2, 0xF3, 0xF5),   // surfaceVariant #F2F3F5
    QColor(0xE5, 0xE6, 0xEB),   // border       #E5E6EB
    QColor(0xC9, 0xCD, 0xD4),   // borderStrong #C9CDD4
    QColor(0x1D, 0x21, 0x29),   // text         #1D2129
    QColor(0x4E, 0x59, 0x69),   // textSecondary #4E5969
    QColor(0x86, 0x90, 0x9C),   // textTertiary #86909C
    QColor(0xC9, 0xCD, 0xD4),   // textDisabled #C9CDD4
    QColor(0xFF, 0xFF, 0xFF),   // textOnPrimary
    QColor(0x29, 0x29, 0x2E),   // inverseSurface #29292E
    QColor(0xFF, 0xFF, 0xFF),   // inverseText
};

// 暗色:背景/表面/描边取 Arco 暗色灰阶的近似值
inline const Palette kDarkPalette = {
    QColor(0x3C, 0x7F, 0xFF),   // primary
    QColor(0x5C, 0x8D, 0xFF),   // primaryHover
    QColor(0x22, 0x55, 0xCC),   // primaryPressed
    QColor(0x23, 0xC3, 0x43),   // success
    QColor(0xFF, 0x9A, 0x2E),   // warning
    QColor(0xF7, 0x65, 0x60),   // danger
    QColor(0x17, 0x17, 0x1A),   // background
    QColor(0x23, 0x23, 0x24),   // surface
    QColor(0x2E, 0x2E, 0x30),   // surfaceVariant
    QColor(0x3A, 0x3A, 0x3D),   // border
    QColor(0x52, 0x52, 0x57),   // borderStrong
    QColor(0xF7, 0xF8, 0xFA),   // text
    QColor(0xA9, 0xAE, 0xB8),   // textSecondary
    QColor(0x86, 0x90, 0x9C),   // textTertiary
    QColor(0x61, 0x61, 0x6D),   // textDisabled
    QColor(0xFF, 0xFF, 0xFF),   // textOnPrimary
    QColor(0x2E, 0x2E, 0x30),   // inverseSurface
    QColor(0xF7, 0xF8, 0xFA),   // inverseText
};

// 圆角(Fluent 桌面惯例)
enum class Radius : int
{
    SM = 4,   // 小控件(菜单项、列表项)
    MD = 6,   // 按钮、输入框
    LG = 8,   // 卡片、弹窗
};

// 间距(4 的倍数阶梯)
enum class Spacing : int
{
    XS = 4,
    SM = 8,
    MD = 12,
    LG = 16,
    XL = 24,
};

// 动效时长
enum class Duration : int
{
    Fast = 150,    // 悬浮/按下反馈
    Normal = 250,  // 常规过渡(渐变、位移)
    Slow = 400,    // 大面积过渡
};

// 字号(像素)
enum class FontSize : int
{
    Caption = 12,      // 辅助说明
    Body = 13,         // 正文(控件默认)
    BodyStrong = 14,   // 强调正文
    Subtitle = 16,     // 小标题
    Title = 20,        // 标题
};

// 控件标准高度
enum class ControlHeight : int
{
    SM = 28,
    MD = 32,
    LG = 36,
};

} // namespace Tokens
