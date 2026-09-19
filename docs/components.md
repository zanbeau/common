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
Log::info() << "loaded" << n;
Log::setFile("app.log");   // 之后所有日志(含 Qt 自身 qDebug 等)镜像写入文件
```

级别:`debug/info/warning/error`。状态全部藏在 .cpp,对外只有 `namespace Log` 的自由函数。

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

客户区拖动、八方向边缘拉伸(`setResizeMargin()`,默认 5px)、双击最大化/还原。优先走窗口系统 `startSystemMove`/`startSystemResize`(原生贴边、Wayland 兼容),不支持时回退手动实现。

### `FramelessDialog` — 无边框对话框基类

与 FramelessWidget 完全同一套行为,基类是 QDialog,供"关于/确认/设置"弹窗用;标题栏内容由使用方摆放。

### `FramelessHandler` — 无边框行为复用

上述两个类的行为本体(事件过滤器)。任何顶层窗口都能挂:

```cpp
// 让任意 QWidget 窗口获得拖动/拉伸/双击最大化
setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
new FramelessHandler(this, this);
```

### `TitleBar` — 标题栏

无边框窗口的标题栏:标题(左)+ 最小化/最大化/关闭(右),主题化绘制。本体对鼠标透明,按下即拖动、双击即最大化(事件穿透给 FramelessHandler),只有三个按钮自己响应点击;关闭时先发 `closeRequested()` 再关窗:

```cpp
auto *bar = new TitleBar(this, QStringLiteral("我的播放器"));
layout->addWidget(bar);              // 放窗口顶部
connect(bar, &TitleBar::closeRequested, this, &Player::onCloseRequested);
```

### `MessageBox` — 消息框

风格化弹窗(FramelessDialog + TitleBar + 主题按钮 + 语义色图标),用法对齐 QMessageBox:

```cpp
MessageBox::information(this, "提示", "已保存");
if(MessageBox::question(this, "删除", "确定移除这首歌吗?")) { ... }
```

也可直接构造后自行 `exec()`/`show()`;Esc 与标题栏关闭按取消处理。

### `NotifyWindow` — 桌面通知

主屏右下角弹出,自动淡出销毁,多条自下而上堆叠,点击关闭:

```cpp
NotifyWindow::showMessage(QStringLiteral("正在播放"), QStringLiteral("晴天 - 周杰伦"));
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
