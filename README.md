# common

个人 Qt 项目的公共基础库:core(无 UI 依赖)/ widgets(通用控件)。
控件设计参考了 [TTKCommon](https://github.com/Greedysky/TTKWidgetTools)(LGPLv3)的思路,均为自行实现。

## 模块

| 模块 | 内容 |
|---|---|
| core/base | `singleton.h` — CRTP 单例基类 |
| widgets/frameless | `FramelessWidget` — 无边框窗口基类:客户区拖动、八方向边缘拉伸、双击最大化/还原;优先走窗口系统 `startSystemMove`/`startSystemResize`(原生贴边、Wayland 兼容),不支持时回退手动实现 |
| widgets/controls | `ClickedLabel` — 可点击 QLabel,拦截按下事件不向上传播,适合放在无边框标题栏 |
| widgets/controls | `ToastLabel` — 自动淡出的气泡提示,一行调用 `ToastLabel::showText("提示", this)` |

## CMake 接入(FetchContent)

```cmake
include(FetchContent)
FetchContent_Declare(common
    GIT_REPOSITORY https://github.com/zanbeau/common.git
    GIT_TAG        v0.3.0   # 建议锁定版本
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

```sh
cmake -S . -B build          # 需在 VS x64 环境下,Qt 通过 CMAKE_PREFIX_PATH 指定
cmake --build build
ctest --test-dir build       # tests/(offscreen 平台)、examples/ 仅在单独构建时编译
```
