#pragma once

#include <QObject>

#include "tokens.h"

class QColor;
class QString;

// 主题引擎:持有当前亮/暗模式,把语义角色解析为具体取值
//  - color(Role)          —— 语义色(随模式切换)
//  - radius/spacing/...   —— 尺寸/动效令牌的透出(模式无关)
//  - apply()              —— 生成全局 QSS + 同步 QPalette 应用到整个程序
// 控件一律从这里取色并连接 modeChanged() 重绘,即可自动跟随主题切换。
// 便捷用法:
//   Theme::instance()->apply();                          // 程序启动时
//   Theme::instance()->setMode(Theme::Mode::Dark);       // 切换(已 apply 时立即生效)
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
    void setMode(Mode mode);   // 已 apply() 过则立即刷新全局样式

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
    void modeChanged(Theme::Mode mode);

private:
    explicit Theme(QObject *parent = nullptr);

    Mode m_mode = Mode::Light;
    bool m_applied = false;
};
