# common

个人 Qt 项目的公共基础库:core(无 UI 依赖)/ widgets(通用控件)。
一套源码同时支持 **Qt 5.15 与 Qt 6**(C++17)。
控件设计参考了 [TTKCommon](https://github.com/Greedysky/TTKWidgetTools)(LGPLv3)的思路,均为自行实现。

## 模块

| 模块 | 内容 |
|---|---|
| core/base | `singleton.h` — CRTP 单例基类 |
| core/base | `logger` — 流式日志 `Log::info() << "loaded" << n;`,`Log::setFile(path)` 后所有日志(含 Qt 自身的 qDebug 等)镜像写入文件 |
| core/app | `SingleInstance` — 单实例守护(防双开):组合式设计不绑定应用类;副实例 `sendMessage()` 通知主实例并等待确认后退出 |
| widgets/frameless | `FramelessWidget` — 无边框窗口基类:客户区拖动、八方向边缘拉伸、双击最大化/还原;优先走窗口系统 `startSystemMove`/`startSystemResize`(原生贴边、Wayland 兼容),不支持时回退手动实现 |
| widgets/controls | `ClickedLabel` — 可点击 QLabel,拦截按下事件不向上传播,适合放在无边框标题栏 |
| widgets/controls | `ToastLabel` — 自动淡出的气泡提示,一行调用 `ToastLabel::showText("提示", this)` |

core 依赖 Qt::Core + Qt::Network(QLocalServer/QLocalSocket),使用方 `find_package(...)` 需包含 `Network` 组件;qmake 接入无需额外处理(common.pri 已加 `QT += network`)。

单实例典型用法:

```cpp
QApplication app(argc, argv);
SingleInstance guard(QStringLiteral("my-app-key"));
if(!guard.isPrimary())
{
    guard.sendMessage(QStringLiteral("activate")); // 通知已有实例后退出
    return 0;
}
QObject::connect(&guard, &SingleInstance::messageReceived, &window, [&window]() {
    window.showNormal(); window.raise(); window.activateWindow();
});
```

## CMake 接入(FetchContent)

```cmake
include(FetchContent)
FetchContent_Declare(common
    GIT_REPOSITORY https://github.com/zanbeau/common.git
    GIT_TAG        v0.4.0   # 建议锁定版本
)
FetchContent_MakeAvailable(common)

target_link_libraries(your_app PRIVATE canfan::widgets)
```

`#include "framelesswidget.h"` 与 `#include "frameless/framelesswidget.h"` 两种写法都可用。

## qmake 接入(.pro)

```qmake
include(<common 路径>/common.pri)   # 源码直接编入工程,无需先构建库
```

也可 qmake 独立构建静态库:`qmake common.pro && nmake / mingw32-make`。

## 构建与测试

通过 `CMAKE_PREFIX_PATH` 选择 Qt 版本,同一份源码双版本编译:

```sh
# Qt 6
cmake -S . -B build -DCMAKE_PREFIX_PATH=D:/Qt/6.8.3/msvc2022_64
cmake --build build
ctest --test-dir build

# Qt 5.15
cmake -S . -B build-qt5 -DCMAKE_PREFIX_PATH=D:/Qt/5.15.2/msvc2019_64
cmake --build build-qt5
ctest --test-dir build-qt5   # Windows 运行测试前需把 Qt 的 bin 目录加入 PATH
```

需在 VS x64 环境下构建;tests/(offscreen 平台)、examples/ 仅在单独构建时编译。
构建侧统一设置 `QT_DISABLE_DEPRECATED_BEFORE=0x050F00`,禁止使用 Qt 5.15 之前已弃用的 API,保证双版本长期干净编译。
