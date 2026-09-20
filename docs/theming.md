# 主题与设计令牌

common 的视觉体系由两层构成:

- **`Tokens`**(`widgets/theme/tokens.h`)—— 设计变量唯一来源:语义色板(亮/暗各一套)、圆角/间距/动效/字号/控件高度常量
- **`Theme`**(`widgets/theme/theme.h`)—— 主题引擎:按当前模式把语义角色解析成具体取值,生成全局 QSS,广播主题切换

设计基调:色板与语义色参考 [Arco Design](https://arco.design)(主色 arcoblue `#165DFF`),圆角(4/6/8px)与桌面交互惯例参考 Fluent。控件只消费语义角色,不出现裸色值——这是硬约定,写进 CLAUDE.md。

## 快速开始

```cpp
QApplication app(argc, argv);

Theme::instance()->apply();                    // 启动时应用:全局 QSS + QPalette
// ... 用户在设置里点"暗色":
Theme::instance()->setMode(Theme::Mode::Dark); // 即时切换,所有吃令牌的控件自动重绘
```

`apply()` 覆盖的基础件:`QPushButton`、`QLineEdit`/`QPlainTextEdit`/`QTextEdit`、`QListView`/`QListWidget`、`QMenu`、`QToolTip`、`QSlider`、`QComboBox`(含下拉面板)、`QCheckBox`/`QRadioButton`、`QScrollBar`、`QTabWidget`/`QTabBar`、`QTableView`/`QHeaderView`、`QProgressBar`,以及通过 QPalette 联动的 `QLabel` 等原生件。库内自绘控件(PushButton、ToggleSwitch 等)不依赖全局 QSS,直接读令牌。

## 语义色角色

`Theme::color(Role)` 返回当前模式下的颜色:

| Role | 亮色 | 暗色 | 用途 |
|---|---|---|---|
| `Primary` / `PrimaryHover` / `PrimaryPressed` | `#165DFF` / `#4080FF` / `#0E42D2` | `#3C7FFF` / `#5C8DFF` / `#2255CC` | 主色及其悬浮/按下态 |
| `Success` / `Warning` / `Danger` | Arco green/orange/red-6 | 对应暗色近似值 | 状态语义色 |
| `Background` | `#F7F8FA` | `#17171A` | 窗口底色 |
| `Surface` | `#FFFFFF` | `#232324` | 控件/卡片表面 |
| `SurfaceVariant` | `#F2F3F5` | `#2E2E30` | 次级表面(悬浮底、禁用底) |
| `Border` / `BorderStrong` | `#E5E6EB` / `#C9CDD4` | `#3A3A3D` / `#525257` | 常规/强描边 |
| `Text` / `TextSecondary` / `TextTertiary` / `TextDisabled` | Arco 灰阶 10/7/5/4 | 对应暗色近似值 | 文本层级 |
| `TextOnPrimary` | `#FFFFFF` | `#FFFFFF` | 主色底上的文字 |
| `InverseSurface` / `InverseText` | `#29292E` / `#FFFFFF` | `#2E2E30` / `#F7F8FA` | 反色表面(Toast、Tooltip) |

## 尺寸与动效令牌

模式无关,直接透出为 `int`:

| 族 | API | 值 |
|---|---|---|
| 圆角 | `Theme::radius(Tokens::Radius::SM/MD/LG)` | 4 / 6 / 8 px |
| 间距 | `Theme::spacing(Tokens::Spacing::XS..XL)` | 4 / 8 / 12 / 16 / 24 px |
| 动效 | `Theme::duration(Tokens::Duration::Fast/Normal/Slow)` | 150 / 250 / 400 ms |
| 字号 | `Theme::fontPx(Tokens::FontSize::Caption..Title)` | 12 / 13 / 14 / 16 / 20 px |
| 控件高度 | `Theme::controlHeight(Tokens::ControlHeight::SM/MD/LG)` | 28 / 32 / 36 px |

另有 `Theme::mode()` 读取当前亮暗模式(不触发重刷)。

## 给自绘控件接入主题

标准模式(库内所有自绘控件都这么做):

```cpp
MyWidget::MyWidget(QWidget *parent) : QWidget(parent)
{
    // 主题切换时重绘
    connect(Theme::instance(), &Theme::modeChanged, this, [this]() { update(); });
}

void MyWidget::paintEvent(QPaintEvent *)
{
    Theme *theme = Theme::instance();
    QPainter painter(this);
    painter.setPen(theme->color(Theme::Role::Text));
    // ... 不要出现任何写死的十六进制色值
}
```

## 模式持久化(应用侧)

`Theme` 不碰 QSettings(库不做持久化决策);应用侧三行即可:

```cpp
// 启动时
Theme::instance()->setMode(settings.value("theme/dark").toBool()
                               ? Theme::Mode::Dark : Theme::Mode::Light);
Theme::instance()->apply();

// 切换时
connect(toggle, &ToggleSwitch::toggled, this, [&](bool dark) {
    Theme::instance()->setMode(dark ? Theme::Mode::Dark : Theme::Mode::Light);
    settings.setValue("theme/dark", dark);
});
```

注意先 `setMode` 再 `apply`:`apply()` 之后 `setMode` 会立即重刷全局样式,顺序不影响结果,但启动时一次到位更干净。

## 扩展指南

- **新增语义角色**:在 `Theme::Role` 加枚举 → `Tokens::Palette` 加字段并补进两套色板 → `theme.cpp` 的 `resolve()` 加一行
- **换品牌色**:改 `Tokens::kLightPalette/kDarkPalette` 的 `primary*` 三个字段;主色相关的悬浮/按下态、选中项、焦点边框都会跟着走
- **扩大 QSS 覆盖**:在 `Theme::styleSheet()` 里追加选择器;新控件如果 QSS 能表达就优先用全局 QSS,表达不了(需要动画/复杂态)才走自绘
