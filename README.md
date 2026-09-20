# common

个人 Qt 项目的组件库与公共基础库:core(无 UI 依赖)/ widgets(主题化控件)。
一套源码同时支持 **Qt 5.15 与 Qt 6**(C++17),自带亮/暗双主题的设计令牌体系。
控件设计参考了 [TTKCommon](https://github.com/Greedysky/TTKWidgetTools)(LGPLv3)、
[Arco Design](https://arco.design) 色板与 Fluent 的桌面惯例,均为自行实现。

## 文档

- **[docs/components.md](docs/components.md)** — 全组件参考(用法示例)
- **[docs/theming.md](docs/theming.md)** — 主题与设计令牌指南

## 模块概览

| 层 | 类别 | 组件 |
|---|---|---|
| core | base | `singleton` / `logger` / `duration` |
| core | application | `SingleInstance`(防双开) |
| widgets | theme | `Tokens` / `Theme`(设计令牌 + 主题引擎) |
| widgets | button | `PushButton` / `IconButton` / `ToggleSwitch` |
| widgets | input | `SearchInput` |
| widgets | label | `ClickedLabel` / `MarqueeLabel` / `ToastLabel` / `TransitionLabel` / `RotateLabel` |
| widgets | slider | `ClickedSlider` / `TipSlider` |
| widgets | progress | `WaitSpinner` |
| widgets | widget | `AnimationStackedWidget` / `SideNav` / `CoverFlow` |
| widgets | window | `FramelessWidget` / `FramelessDialog` / `FramelessHandler` / `TitleBar` / `MessageBox` / `NotifyWindow` / `SplashScreen` |

主题三行起步:

```cpp
QApplication app(argc, argv);
Theme::instance()->apply();                      // 启动时应用主题(全局 QSS + QPalette)
Theme::instance()->setMode(Theme::Mode::Dark);   // 即时切换,吃令牌的控件自动重绘
// 可选:setAccent() 覆盖主色族(强调色),setFollowSystem() 跟随系统亮暗
```

core 依赖 Qt::Core + Qt::Network(QLocalServer/QLocalSocket),使用方 `find_package(...)` 需包含 `Network` 组件;qmake 接入无需额外处理(common.pri 已加 `QT += network`)。

## CMake 接入(FetchContent)

```cmake
include(FetchContent)
FetchContent_Declare(common
    GIT_REPOSITORY https://github.com/zanbeau/common.git
    GIT_TAG        v0.10.0   # 建议锁定版本
)
FetchContent_MakeAvailable(common)

target_link_libraries(your_app PRIVATE canfan::widgets)
```

每个类别目录都在 include 路径里,`#include "clickedlabel.h"` 与 `#include "label/clickedlabel.h"` 两种写法都可用。

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

examples/ 是控件浏览器(gallery):每类控件一页演示,`widgets_demo.exe` 打开即见,第一页可切换亮暗主题看全局换肤。
