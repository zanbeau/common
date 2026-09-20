#include "theme.h"

#include <QApplication>
#include <QGuiApplication>
#include <QPalette>
#include <QSettings>

#ifdef Q_OS_WIN
#define NOMINMAX
#include <windows.h>
#include <cwchar>
#include <QAbstractNativeEventFilter>
#endif

namespace {

#ifdef Q_OS_WIN
// 监听系统亮暗切换:Personalize\AppsUseLightTheme 变化时系统广播
// WM_SETTINGCHANGE("ImmersiveColorSet"),这里转成 Theme 的跟随刷新
class SystemThemeFilter : public QAbstractNativeEventFilter
{
public:
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override
#else
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override
#endif
    {
        Q_UNUSED(result)
        if(eventType == QByteArrayLiteral("windows_generic_MSG"))
        {
            const MSG *msg = static_cast<const MSG *>(message);
            if(msg->message == WM_SETTINGCHANGE && msg->lParam
               && wcscmp(reinterpret_cast<const wchar_t *>(msg->lParam), L"ImmersiveColorSet") == 0)
            {
                Theme::instance()->syncWithSystem();
            }
        }
        return false;
    }
};
SystemThemeFilter *g_systemThemeFilter = nullptr;
#endif

// 角色 -> 色板字段
QColor resolve(const Tokens::Palette &pal, Theme::Role role)
{
    switch(role)
    {
    case Theme::Role::Primary:        return pal.primary;
    case Theme::Role::PrimaryHover:   return pal.primaryHover;
    case Theme::Role::PrimaryPressed: return pal.primaryPressed;
    case Theme::Role::Success:        return pal.success;
    case Theme::Role::Warning:        return pal.warning;
    case Theme::Role::Danger:         return pal.danger;
    case Theme::Role::Background:     return pal.background;
    case Theme::Role::Surface:        return pal.surface;
    case Theme::Role::SurfaceVariant: return pal.surfaceVariant;
    case Theme::Role::Border:         return pal.border;
    case Theme::Role::BorderStrong:   return pal.borderStrong;
    case Theme::Role::Text:           return pal.text;
    case Theme::Role::TextSecondary:  return pal.textSecondary;
    case Theme::Role::TextTertiary:   return pal.textTertiary;
    case Theme::Role::TextDisabled:   return pal.textDisabled;
    case Theme::Role::TextOnPrimary:  return pal.textOnPrimary;
    case Theme::Role::InverseSurface: return pal.inverseSurface;
    case Theme::Role::InverseText:    return pal.inverseText;
    }
    return pal.text;
}

} // namespace

Theme *Theme::instance()
{
    static Theme inst;
    return &inst;
}

Theme::Theme(QObject *parent)
    : QObject(parent)
{
}

Theme::Mode Theme::mode() const
{
    return m_mode;
}

void Theme::setMode(Mode mode)
{
    // 手动选择优先:一旦显式 setMode,停止跟随系统
    m_followSystem = false;
    if(m_mode == mode)
    {
        return;
    }
    updateMode(mode);
}

void Theme::updateMode(Mode mode)
{
    m_mode = mode;
    if(m_applied)
    {
        apply();
    }
    emit modeChanged(mode);
    emit themeChanged();
}

QColor Theme::accent() const
{
    return m_accent;
}

void Theme::setAccent(const QColor &color)
{
    if(m_accent == color)
    {
        return;
    }
    m_accent = color;
    if(m_applied)
    {
        apply();
    }
    emit themeChanged();
}

bool Theme::followSystem() const
{
    return m_followSystem;
}

void Theme::setFollowSystem(bool follow)
{
    if(m_followSystem == follow)
    {
        return;
    }
    m_followSystem = follow;
#ifdef Q_OS_WIN
    if(follow && !g_systemThemeFilter)
    {
        g_systemThemeFilter = new SystemThemeFilter;
        qApp->installNativeEventFilter(g_systemThemeFilter);
    }
#endif
    if(follow)
    {
        syncWithSystem();
    }
}

Theme::Mode Theme::systemMode() const
{
#ifdef Q_OS_WIN
    // 注册表 AppsUseLightTheme(0 = 系统暗色);键不存在按亮色处理
    const QSettings settings(
        QStringLiteral("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize"),
        QSettings::NativeFormat);
    return settings.value(QStringLiteral("AppsUseLightTheme"), 1).toInt() == 0
               ? Mode::Dark
               : Mode::Light;
#else
    // 退路:按应用调色板窗口底的明度判断(深色 GTK/QPA 主题)
    return QGuiApplication::palette().color(QPalette::Window).lightness() < 128
               ? Mode::Dark
               : Mode::Light;
#endif
}

void Theme::syncWithSystem()
{
    if(!m_followSystem)
    {
        return;
    }
    const Mode system = systemMode();
    if(m_mode != system)
    {
        updateMode(system);
    }
}

QColor Theme::color(Role role) const
{
    // 强调色覆盖:主色族四个角色由强调色推导(hover 提亮/按下加深,
    // 暗色下提亮幅度更大),其余角色不受影响
    if(m_accent.isValid())
    {
        switch(role)
        {
        case Role::Primary:
            return m_accent;
        case Role::PrimaryHover:
            return m_mode == Mode::Dark ? m_accent.lighter(130) : m_accent.lighter(112);
        case Role::PrimaryPressed:
            return m_mode == Mode::Dark ? m_accent.darker(108) : m_accent.darker(112);
        case Role::TextOnPrimary:
        {
            // 按相对亮度决定主色上的文字用深还是浅
            const qreal luminance = 0.299 * m_accent.redF() + 0.587 * m_accent.greenF()
                                  + 0.114 * m_accent.blueF();
            return luminance > 0.65 ? QColor(0x1D, 0x21, 0x29) : QColor(Qt::white);
        }
        default:
            break;
        }
    }
    return m_mode == Mode::Dark ? resolve(Tokens::kDarkPalette, role)
                                : resolve(Tokens::kLightPalette, role);
}

QString Theme::styleSheet() const
{
    // 分节组合:每节独立的 .arg 链,避免一整条 %1..%n 的脆弱映射
    const QString surface = color(Role::Surface).name();
    const QString surfaceVariant = color(Role::SurfaceVariant).name();
    const QString border = color(Role::Border).name();
    const QString borderStrong = color(Role::BorderStrong).name();
    const QString text = color(Role::Text).name();
    const QString textSecondary = color(Role::TextSecondary).name();
    const QString textDisabled = color(Role::TextDisabled).name();
    const QString primary = color(Role::Primary).name();
    const QString onPrimary = color(Role::TextOnPrimary).name();
    const QString inverseSurface = color(Role::InverseSurface).name();
    const QString inverseText = color(Role::InverseText).name();
    const int sm = radius(Tokens::Radius::SM);
    const int md = radius(Tokens::Radius::MD);
    const int body = fontPx(Tokens::FontSize::Body);

    // 覆盖应用里的基础件;控件库自绘控件不依赖这里的 QSS
    QString sheet;

    sheet += QStringLiteral(
        "QPushButton {"
        "  background-color: %1; border: 1px solid %2; border-radius: %3px;"
        "  padding: 5px 16px; color: %4; font-size: %5px;"
        "}"
        "QPushButton:hover { background-color: %6; border-color: %7; }"
        "QPushButton:pressed { background-color: %8; }"
        "QPushButton:disabled { color: %9; background-color: %6; border-color: %2; }")
        .arg(surface, border)
        .arg(md)
        .arg(text)
        .arg(body)
        .arg(surfaceVariant, borderStrong, border)
        .arg(textDisabled);

    sheet += QStringLiteral(
        "QLineEdit, QPlainTextEdit, QTextEdit {"
        "  background-color: %1; border: 1px solid %2; border-radius: %3px;"
        "  padding: 5px 10px; color: %4; font-size: %5px;"
        "  selection-background-color: %6; selection-color: %7;"
        "}"
        "QLineEdit:hover, QPlainTextEdit:hover, QTextEdit:hover { border-color: %8; }"
        // 聚焦边框加粗到 2px 主色;内边距各减 1px 抵消增粗,文字与图标不跳动
        "QLineEdit:focus, QPlainTextEdit:focus, QTextEdit:focus { border: 2px solid %6; padding: 4px 9px; }"
        "QLineEdit:disabled, QPlainTextEdit:disabled, QTextEdit:disabled { color: %9; background-color: %10; }")
        .arg(surface, border)
        .arg(md)
        .arg(text)
        .arg(body)
        .arg(primary, onPrimary, borderStrong, textDisabled, surfaceVariant);

    sheet += QStringLiteral(
        "QListView, QListWidget {"
        "  background-color: %1; border: 1px solid %2; border-radius: %3px; color: %4;"
        "  outline: none;"
        "}"
        "QListView::item, QListWidget::item { padding: 6px 10px; border-radius: %5px; color: %4; }"
        "QListView::item:hover, QListWidget::item:hover { background-color: %6; }"
        "QListView::item:selected, QListWidget::item:selected { background-color: %7; color: %8; }")
        .arg(surface, border)
        .arg(md)
        .arg(text)
        .arg(sm)
        .arg(surfaceVariant, primary, onPrimary);

    sheet += QStringLiteral(
        "QMenu { background-color: %1; border: 1px solid %2; border-radius: %3px; padding: 4px; color: %4; }"
        "QMenu::item { padding: 6px 24px; border-radius: %5px; }"
        "QMenu::item:selected { background-color: %6; }"
        "QMenu::separator { height: 1px; background-color: %2; margin: 4px 8px; }"
        "QToolTip { background-color: %7; color: %8; border: none; padding: 4px 8px; }")
        .arg(surface, border)
        .arg(md)
        .arg(text)
        .arg(sm)
        .arg(surfaceVariant)
        .arg(inverseSurface, inverseText);

    // 滑条:细轨 + 主色已播段 + 白心主色描边的手柄
    sheet += QStringLiteral(
        "QSlider::groove:horizontal { height: 4px; border-radius: 2px; background: %1; }"
        "QSlider::sub-page:horizontal { background: %2; border-radius: 2px; }"
        "QSlider::handle:horizontal { width: 12px; height: 12px; margin: -5px 0;"
        "  border-radius: 6px; background: %3; border: 2px solid %2; }"
        "QSlider::groove:vertical { width: 4px; border-radius: 2px; background: %1; }"
        "QSlider::sub-page:vertical { background: %2; border-radius: 2px; }"
        "QSlider::handle:vertical { width: 12px; height: 12px; margin: 0 -5px;"
        "  border-radius: 6px; background: %3; border: 2px solid %2; }"
        "QSlider::sub-page:disabled { background: %4; }")
        .arg(border, primary, onPrimary)
        .arg(borderStrong);

    // 下拉框:箭头用 QSS 边框三角形拼出来(不引图标资源)
    sheet += QStringLiteral(
        "QComboBox {"
        "  background-color: %1; border: 1px solid %2; border-radius: %3px;"
        "  padding: 5px 12px 5px 10px; color: %4; font-size: %5px; min-width: 80px;"
        "}"
        "QComboBox:hover { border-color: %6; }"
        "QComboBox:focus { border-color: %7; }"
        "QComboBox:disabled { color: %8; background-color: %9; }"
        "QComboBox::drop-down { border: none; width: 22px; }"
        "QComboBox::down-arrow { width: 0; height: 0;"
        "  border-left: 4px solid transparent; border-right: 4px solid transparent;"
        "  border-top: 5px solid %10; }"
        "QComboBox QAbstractItemView {"
        "  background-color: %1; border: 1px solid %2; border-radius: %3px;"
        "  selection-background-color: %7; selection-color: %11; outline: none; }")
        .arg(surface, border)
        .arg(md)
        .arg(text)
        .arg(body)
        .arg(borderStrong, primary, textDisabled, surfaceVariant, textSecondary)
        .arg(onPrimary);

    // 复选/单选:指示器无图标方案——选中即主色实心
    sheet += QStringLiteral(
        "QCheckBox, QRadioButton { color: %1; spacing: 6px; }"
        "QCheckBox::indicator, QRadioButton::indicator { width: 16px; height: 16px; }"
        "QCheckBox::indicator { border: 1px solid %2; border-radius: 3px; background: %3; }"
        "QCheckBox::indicator:checked { background: %4; border-color: %4; }"
        "QRadioButton::indicator { border: 1px solid %2; border-radius: 8px; background: %3; }"
        "QRadioButton::indicator:checked { background: %4; border-color: %4; }"
        "QCheckBox::indicator:disabled, QRadioButton::indicator:disabled {"
        "  border-color: %5; background: %6; }")
        .arg(text, borderStrong, surface, primary, border, surfaceVariant);

    // 滚动条:细条 + 圆角滑块,无箭头按钮
    sheet += QStringLiteral(
        "QScrollBar:vertical { background: transparent; width: 10px; margin: 2px; }"
        "QScrollBar::handle:vertical { background: %1; border-radius: 4px; min-height: 30px; }"
        "QScrollBar::handle:vertical:hover { background: %2; }"
        "QScrollBar:horizontal { background: transparent; height: 10px; margin: 2px; }"
        "QScrollBar::handle:horizontal { background: %1; border-radius: 4px; min-width: 30px; }"
        "QScrollBar::handle:horizontal:hover { background: %2; }"
        "QScrollBar::add-line, QScrollBar::sub-line { width: 0; height: 0; }"
        "QScrollBar::add-page, QScrollBar::sub-page { background: transparent; }")
        .arg(border, borderStrong);

    sheet += QStringLiteral(
        "QTabWidget::pane { border: 1px solid %1; border-radius: %2px; }"
        "QTabBar::tab { padding: 6px 14px; color: %3; }"
        "QTabBar::tab:selected { color: %4; border-bottom: 2px solid %5; }"
        "QTabBar::tab:!selected { color: %3; }")
        .arg(border)
        .arg(md)
        .arg(textSecondary, text, primary);

    // 表格:去系统网格线,选中行主色底
    sheet += QStringLiteral(
        "QTableView {"
        "  background-color: %1; border: 1px solid %2; border-radius: %3px;"
        "  gridline-color: %2; outline: none; color: %4; font-size: %5px;"
        "  alternate-background-color: %6; selection-background-color: %7;"
        "  selection-color: %8;"
        "}"
        "QTableView::item { padding: 5px 8px; border: none; }"
        "QTableView::item:hover { background: %9; }"
        "QHeaderView { background: transparent; border: none; }"
        "QHeaderView::section {"
        "  background-color: %10; color: %11; border: none;"
        "  border-bottom: 1px solid %2; padding: 6px 8px; font-weight: bold;"
        "}"
        "QTableCornerButton::section { background-color: %10; border: none; }")
        .arg(surface, border)
        .arg(md)
        .arg(text)
        .arg(body)
        .arg(surfaceVariant, primary, onPrimary, surfaceVariant)
        .arg(surface)
        .arg(textSecondary);

    sheet += QStringLiteral(
        "QProgressBar {"
        "  border: none; background-color: %1; border-radius: 4px;"
        "  text-align: center; color: %2; font-size: %3px;"
        "}"
        "QProgressBar::chunk { background-color: %4; border-radius: 4px; }")
        .arg(surfaceVariant)
        .arg(text)
        .arg(body)
        .arg(primary);

    return sheet;
}

void Theme::apply()
{
    if(!qApp)
    {
        return;
    }
    m_applied = true;

    // QPalette 同步:让未覆盖 QSS 的原生件(QLabel、QToolTip 等)也跟随主题。
    // 顺序很重要:必须先调色板后样式表——设置样式表会让全部控件重新 polish
    // 并快照当前调色板,而 QSS 下之后的调色板变更不会刷新已有控件;
    // 先设调色板,重 polish 才能拿到新值,否则所有调色板件慢一拍(白底白字)。
    QPalette pal;
    pal.setColor(QPalette::Window, color(Role::Background));
    pal.setColor(QPalette::WindowText, color(Role::Text));
    pal.setColor(QPalette::Base, color(Role::Surface));
    pal.setColor(QPalette::AlternateBase, color(Role::SurfaceVariant));
    pal.setColor(QPalette::Text, color(Role::Text));
    pal.setColor(QPalette::Button, color(Role::Surface));
    pal.setColor(QPalette::ButtonText, color(Role::Text));
    pal.setColor(QPalette::ToolTipBase, color(Role::InverseSurface));
    pal.setColor(QPalette::ToolTipText, color(Role::InverseText));
    pal.setColor(QPalette::Highlight, color(Role::Primary));
    pal.setColor(QPalette::HighlightedText, color(Role::TextOnPrimary));
    pal.setColor(QPalette::PlaceholderText, color(Role::TextTertiary));
    pal.setColor(QPalette::Disabled, QPalette::Text, color(Role::TextDisabled));
    pal.setColor(QPalette::Disabled, QPalette::WindowText, color(Role::TextDisabled));
    pal.setColor(QPalette::Disabled, QPalette::ButtonText, color(Role::TextDisabled));
    qApp->setPalette(pal);

    qApp->setStyleSheet(styleSheet());
}
