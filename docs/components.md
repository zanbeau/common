# 组件参考

目录名即类别:core 下是 `base/`(基础件)、`application/`(应用级设施);widgets 下每个子目录是一类控件。每个类别目录都在 include 路径里,`#include "clickedlabel.h"` 与 `#include "label/clickedlabel.h"` 两种写法都可用。

主题与令牌体系见 [theming.md](theming.md)。

## core/base

### `singleton.h` — CRTP 单例基类

```cpp
class Manager : public Singleton<Manager> { ... };
Manager::instance()->...;
```

函数局部静态实现(C++11 magic static,线程安全),进程存活期常驻。

### `logger` — 流式日志

```cpp
Log::info() << "loaded" << n;            // 自由函数,不带调用点
LOG_INFO << "loaded" << n;               // 宏,镜像文件里带 [文件名(行号)] 前缀
LOG_INFO_ONCE("只打一次" << n);           // 节流族:_ONCE / _COND / _COUNT / _PERIOD
LOG_WARNING_PERIOD(5, "至多 5 秒一条");    // 六个级别 × 四种节流,自由组合

Log::setFile("logs/app.log");  // 之后所有日志(含 Qt 自身 qDebug 等)镜像写入。
                               // 实际文件 logs/app_<日期>_<序号>.log:跨天或单文件
                               // 超过 5MB(Log::setMaxSize)滚动到新文件;打开时清理
                               // 7 天前(Log::setExpireDays)同基准名的旧 .log
Log::setLevel(Log::Level::Warning); // 运行时级别过滤:低于级别的消息直接丢弃
                                    // (流式 API 与 Qt 自身消息两处都拦);默认 Trace
                                    // 全放行,Fatal 恒放行(qFatal 后程序必然中止)
```

级别:`trace/debug/info/warning/error/fatal`(借自 TTK 的六级划分;fatal 走 qFatal
语义,提交后程序中止)。trace 走独立 category,镜像文件里才能与 DEBUG 区分。宏在
调用点捕获 `__FILE__`/`__LINE__`,流式参数用 `<<` 连接,顶层出现逗号需自行加括号
(宏参数切分,与 TTK 相同的限制)。非宏消息(Qt 内部的 qWarning 等)从
`QMessageLogContext` 补调用点。状态全部藏在 .cpp,对外只有 `namespace Log` 的自由
函数 + `LOG_*` 宏。

### `duration.h` — 播放时长格式化

```cpp
Duration::format(65000)     // "01:05";超 1 小时为 "h:mm:ss"
Duration::parse("1:01")     // 61000;非法输入返回 -1
```

## core/application

### `SingleInstance` — 单实例守护

组合式设计,不绑定应用类;副实例 `sendMessage()` 通知主实例并等待确认后退出(换行分帧、阻塞等 ack——不等 ack 的话副实例退出可能丢消息):

```cpp
SingleInstance guard(QStringLiteral("my-app-key"));
if(!guard.isPrimary())
{
    guard.sendMessage(QStringLiteral("activate"));
    return 0;
}
QObject::connect(&guard, &SingleInstance::messageReceived, &window, [&window]() {
    window.showNormal(); window.raise(); window.activateWindow();
});
```

主实例需保持事件循环响应。依赖 Qt::Network(QLocalServer/QLocalSocket)。

## widgets/theme

见 [theming.md](theming.md)。`Tokens` 为设计变量常量,`Theme::instance()` 为解析引擎(`color(Role)` / `apply()` / `setMode()`)。

## widgets/button

### `PushButton` — 主题按钮

四形态:`Default`(白底描边)/ `Primary`(主色实底)/ `Danger`(危险实底)/ `Text`(纯文字)。全自绘,悬浮/按下/禁用/亮暗全部自动联动:

```cpp
auto *ok = new PushButton(QStringLiteral("确定"), PushButton::Type::Primary);
auto *del = new PushButton(QStringLiteral("删除"), PushButton::Type::Danger);
connect(ok, &PushButton::clicked, ...);   // 信号同 QPushButton
```

### `IconButton` — 图标按钮

内置 21 个常用字形(播放/暂停/切歌、音量三态、循环/单曲/随机,以及加减/关闭/勾/箭头/更多/收藏),全部 QPainter 绘制、随主题换色,不需要图标字体或资源文件。悬浮/按下画淡色底;可 checkable(选中态主色淡底 + 主色图标);`setGlyph()` 运行时换图标:

```cpp
auto *play = new IconButton(IconButton::Glyph::Play);
play->setIconSize(24);                       // 默认 16,按钮 32x32
connect(play, &IconButton::clicked, this, [play]() {
    play->setGlyph(play->glyph() == IconButton::Glyph::Play
                       ? IconButton::Glyph::Pause : IconButton::Glyph::Play);   // 播放⇄暂停
});
```

静音这类两态按钮用 checkable + `toggled` 换字形。`paintGlyph()` 是公开静态函数,其他自绘控件(菜单项、列表指示等)可复用同一套字形保持全局风格一致。

### `ToggleSwitch` — 开关

点击或空格切换,滑块位移动效;`toggled(bool)` / `clicked()` 信号,`setChecked()` 同值静默:

```cpp
auto *sw = new ToggleSwitch;
connect(sw, &ToggleSwitch::toggled, this, &Settings::setDarkMode);
```

## widgets/input

### `SearchInput` — 搜索框

前置放大镜图标(随主题换色)+ 内置清除按钮,回车发 `searchRequested(text)`:

```cpp
auto *input = new SearchInput(QStringLiteral("搜索歌曲 / 歌手"));
connect(input, &SearchInput::searchRequested, this, &Library::search);
```

图标 action 可通过 `leadingAction()` 换成自定义图标。

## widgets/label

### `ClickedLabel`

可点击 QLabel,拦截按下事件不向上传播,适合放在无边框标题栏;`clicked()` 信号。

### `MarqueeLabel`

文字超宽时自动横向滚动(hover 暂停、方向可设),不超宽即普通 QLabel。

### `ToastLabel` — 气泡提示

```cpp
ToastLabel::showText(QStringLiteral("已添加到我喜欢"), this);   // 父窗口居中,淡出自毁
```

反色表面配色(深底浅字)取自 Theme,亮暗主题自动联动。

### `TransitionLabel` — 封面渐变

`setPixmap()` 触发新旧图交叉淡化(时长取 `Duration::Normal`),首次设置直接显示:

```cpp
cover->setPixmap(nextCover);   // 一行完成切换动效
```

### `RotateLabel` — 旋转封面

唱片效果:默认静止,`setRunning(true)` 开始转,`setLoopDuration(ms)` 调一圈时长,`setCircular(true)` 圆形裁剪。

## widgets/slider

### `ClickedSlider`

点击轨道任意位置手柄直接跳过去,按住可继续拖动;音量条/进度条用;`clicked()` 信号。

### `TipSlider` — 进度气泡滑条

在 ClickedSlider 基础上,悬停/拖动时指针上方浮出气泡。range 按毫秒设置,默认格式化为 `mm:ss`:

```cpp
auto *slider = new TipSlider(Qt::Horizontal);
slider->setRange(0, 210000);            // 0 .. 3:30
slider->setFormatter([](int ms) { return formatLyricTime(ms); });  // 可选
```

仅横向。气泡是顶层小窗(子控件会被父窗口裁剪)。

## widgets/progress

### `WaitSpinner`

不定进度的转圈指示,`start()/stop()`,`setLoopDuration()/setColor()/setLineWidth()` 可调。默认颜色取主题主色(亮暗联动);`setColor()` 设过自定义色后不再跟随主题。

## widgets/widget

### `AnimationStackedWidget`

页面横向/纵向滑动切换动画的 QStackedWidget;`setCurrentIndexAnimated(i)`,动画期间的新请求被忽略。

### `SideNav` — 侧边导航

Ant Menu 风格的垂直导航(选中项淡主色底 + 左侧指示条,悬浮/亮暗自动联动),点击或上下键切换。与 QStackedWidget 配对即得"左导航 + 右内容"布局:

```cpp
auto *nav = new SideNav;
nav->addItem(QStringLiteral("发现音乐"));
connect(nav, &SideNav::currentChanged, stack, &QStackedWidget::setCurrentIndex);
```

### `CoverFlow` — 封面流

中间大、两侧渐小渐淡、带倒影的封面浏览;滚轮 / 左右键切换,按住拖动翻页,松手滑动吸附:

```cpp
flow->setCovers(covers);
connect(flow, &CoverFlow::currentChanged, this, &Browser::onCoverChanged);
```

## widgets/window

### `FramelessWidget` — 无边框窗口基类

八方向边缘拉伸(`setResizeMargin()`,默认 5px)对全窗口生效;**拖动与双击最大化只发生在标题栏(watch 面板)上**——内容区(控制栏、列表空白处等)的按下不认领也不拖动窗口。优先走窗口系统 `startSystemMove`/`startSystemResize`(原生贴边、Wayland 兼容),不支持时回退手动实现。

窗口要能拖动,摆放 `TitleBar`(或对自拼面板调用 `FramelessHandler::watch()`)即可。

### `FramelessDialog` — 无边框对话框基类

与 FramelessWidget 完全同一套行为,基类是 QDialog,供"关于/确认/设置"弹窗用;标题栏内容由使用方摆放。

### `FramelessHandler` — 无边框行为复用

上述两个类的行为本体(事件过滤器)。窗口本体只处理边缘拉伸;`watch(panel)` 把面板(标题栏)纳入拖动/双击最大化体系。任何顶层窗口都能挂:

```cpp
// 让任意 QWidget 窗口获得 边缘拉伸 + 面板拖动
setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
auto *handler = new FramelessHandler(this, this);
handler->watch(titleBar);   // 自拼面板;用 TitleBar 则它自己会挂
```

### `TitleBar` — 标题栏

无边框窗口的标题栏:标题(左)+ 最小化/最大化/关闭(右),主题化绘制。bar 内部自挂一个 watch 型 `FramelessHandler`,整条栏的按下被它认领——拖动、双击最大化都发生在栏上,三个按钮作为子控件各自消费自己的点击;关闭时先发 `closeRequested()` 再关窗。`setTitle()` 换标题,`setClosable()` 控制关闭按钮显隐:

```cpp
auto *bar = new TitleBar(this, QStringLiteral("我的播放器"));
layout->addWidget(bar);              // 放窗口顶部
connect(bar, &TitleBar::closeRequested, this, &Player::onCloseRequested);
```

### `MessageBox` — 消息框

风格化弹窗(FramelessDialog + TitleBar + 主题按钮 + 语义色图标),用法对齐 QMessageBox:

```cpp
MessageBox::information(this, "提示", "已保存");
MessageBox::warning(this, "警告", "文件未保存");
MessageBox::critical(this, "错误", "无法打开文件");
if(MessageBox::question(this, "删除", "确定移除这首歌吗?")) { ... }
```

四个静态入口对应 `Icon` 的 `Information/Warning/Critical/Question` 四种图标;直接构造时可传 `Icon::None` 不带图标,`setText()` 更新正文。也可构造后自行 `exec()`/`show()`;Esc 与标题栏关闭按取消处理。

### `NotifyWindow` — 桌面通知

主屏右下角弹出,自动淡出销毁(`duration` 控制停留毫秒数,默认 3500),多条自下而上堆叠,点击任意处关闭:

```cpp
NotifyWindow::showMessage(QStringLiteral("正在播放"), QStringLiteral("晴天 - 周杰伦"));
// 想响应点击:showMessage 返回实例,连它的 clicked() 信号
```

### `SplashScreen` — 启动画面

堆分配使用;logo 居中 + 底部状态文字,主窗就绪后 `finish()` 淡出并自毁:

```cpp
auto *splash = new SplashScreen(QPixmap(":/logo.png"));
splash->showMessage(QStringLiteral("正在加载模块…"));
splash->show();
// ... 初始化 ...
splash->finish(&window);
```
