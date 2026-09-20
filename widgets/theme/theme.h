#pragma once

#include <QObject>

#include "tokens.h"

class QColor;
class QString;

// 主题引擎:持有当前亮/暗模式,把语义角色解析为具体取值
//  - color(Role)          —— 语义色(随模式切换,可被 setAccent 覆盖主色族)
//  - radius/spacing/...   —— 尺寸/动效令牌的透出(模式无关)
//  - apply()              —— 生成全局 QSS + 同步 QPalette 应用到整个程序
// 控件一律从这里取色并连接 themeChanged() 重绘,即可自动跟随任何视觉变化。
// 便捷用法:
//   Theme::instance()->apply();                          // 程序启动时
//   Theme::instance()->setMode(Theme::Mode::Dark);       // 切换(已 apply 时立即生效)
//   Theme::instance()->setAccent(QColor("#7C3AED"));     // 覆盖主色族
//   Theme::instance()->setFollowSystem(true);            // 跟随系统亮暗
class Theme : public QObject
{
    Q_OBJECT
public:
    enum class Mode
    {
        Light,
        Dark
    };
    Q_ENUM(Mode)

    // 语义色角色
    enum class Role
    {
        Primary,
        PrimaryHover,
        PrimaryPressed,
        Success,
        Warning,
        Danger,
        Background,
        Surface,
        SurfaceVariant,
        Border,
        BorderStrong,
        Text,
        TextSecondary,
        TextTertiary,
        TextDisabled,
        TextOnPrimary,
        InverseSurface,  // 反色表面(Toast/Tooltip 深底)
        InverseText
    };
    Q_ENUM(Role)

    static Theme *instance();

    Mode mode() const;
    void setMode(Mode mode);   // 已 apply() 过则立即刷新全局样式;手动调用会关闭跟随系统

    // 强调色覆盖:设置后 Primary/PrimaryHover/PrimaryPressed/TextOnPrimary 四个
    // 角色改由该色推导(悬浮提亮、按下加深、按亮度决定主色上的文字黑白),其余角色
    // 走内置色板;传无效 QColor() 恢复内置色板。已 apply() 过则立即刷新
    QColor accent() const;
    void setAccent(const QColor &color);

    // 跟随系统亮暗:开启即按系统当前设置切换,系统再变时自动跟随
    // (Windows 监听 WM_SETTINGCHANGE 实时生效;其他平台在 setFollowSystem()
    // 与 syncWithSystem() 时检测)。手动 setMode() 会关闭跟随
    bool followSystem() const;
    void setFollowSystem(bool follow);
    void syncWithSystem();     // 主动重查一次系统主题(仅 followSystem 开启时生效)

    QColor color(Role role) const;
    int radius(Tokens::Radius r) const { return static_cast<int>(r); }
    int spacing(Tokens::Spacing s) const { return static_cast<int>(s); }
    int duration(Tokens::Duration d) const { return static_cast<int>(d); }
    int fontPx(Tokens::FontSize s) const { return static_cast<int>(s); }
    int controlHeight(Tokens::ControlHeight h) const { return static_cast<int>(h); }

    // 当前模式下的全局 QSS(按钮/输入框/列表/菜单等基础件)
    QString styleSheet() const;
    // 应用到整个应用:qApp 样式表 + QPalette 同步;此后 setMode 会自动重新应用
    void apply();

signals:
    void modeChanged(Theme::Mode mode);   // 仅模式翻转时发出
    void themeChanged();                  // 任何视觉色变化(模式切换/强调色覆盖)后发出;
                                          // 控件重绘一律连这个,自动跟随两种变化

private:
    explicit Theme(QObject *parent = nullptr);

    Mode systemMode() const;              // 当前系统的亮暗(注册表/调色板启发)
    void updateMode(Mode mode);           // setMode/syncWithSystem 共用的落盘+广播

    Mode m_mode = Mode::Light;
    bool m_applied = false;
    bool m_followSystem = false;
    QColor m_accent;                      // 无效 = 未覆盖
};
