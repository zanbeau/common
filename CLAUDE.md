# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

Shared base library (`common`) for personal Qt projects. One source base supports **both Qt 5.15 and Qt 6** (C++17): CMake uses `find_package(QT NAMES Qt6 Qt5 ...)` + `Qt${QT_VERSION_MAJOR}::` targets, qmake relies on its built-in `QT_MAJOR_VERSION`, and `QT_DISABLE_DEPRECATED_BEFORE=0x050F00` is set in both so anything deprecated before 5.15 fails to compile on either version. The only in-source version branches: `globalMousePos()` in `framelesshandler.cpp` (`globalPosition()` vs `globalPos()`) and the High-DPI attributes set before `QApplication` in `examples/main.cpp` / `tests/tst_widgets.cpp` (Qt5-only, `#if QT_VERSION < 6`).

Two static libraries, layered strictly. Directory name = category (TTKModule-style; no hidden grouping rules): core holds `base/` (primitives) and `application/` (app-level facilities); each widgets subdirectory is one widget category (`theme/`, `button/`, `input/`, `label/`, `slider/`, `progress/`, `widget/`, `window/`). New visual components take colors/sizes from `Theme::instance()` (`widgets/theme/`) — never hardcode hex — and connect to its `modeChanged` signal so light/dark switching repaints them.

- `core/` — Qt::Core + Qt::Network (QLocalServer/QLocalSocket), **no UI dependencies**
  - `base/singleton.h` — CRTP singleton
  - `base/logger` — stream logger (`Log::info() << ...`); `Log::setFile()` installs a message handler mirroring ALL Qt messages to a file
  - `base/duration.h` — header-only `Duration::format(ms)` (`mm:ss`/`h:mm:ss`) and `Duration::parse()` for playback times
  - `application/singleinstance` — composition-based single-instance guard (no QApplication subclass, unlike TTK's three-class approach). Secondary instances `sendMessage()` (newline-framed, blocks until the primary acks — the ack is required: without it a secondary exiting right after write can lose the message). Consumer apps must keep the primary's event loop responsive.
- `widgets/` — Qt::Widgets controls, depends on `canfan::core`
  - `theme/tokens.h` — design tokens: Arco-based semantic palettes (light+dark `Palette` structs) plus Fluent-style Radius/Spacing/Duration/FontSize/ControlHeight enums (mode-invariant, plain values)
  - `theme/theme` — the theme engine: `Theme::instance()` (function-local static QObject — call after QApplication exists), `Mode`/`Role` enums, `color(Role)` resolves per mode, size tokens passed through from Tokens; `styleSheet()` generates base-widget QSS, `apply()` sets app stylesheet + QPalette and makes later `setMode()` re-apply immediately; emits `modeChanged`
  - `button/pushbutton` — themed flat button (Default/Primary/Danger/Text), fully custom-painted from Theme roles (hover via `underMouse()` + `WA_Hover`, pressed via `isDown()`, disabled recolor per type)
  - `button/toggleswitch` — custom-painted switch; plain-QWidget subclasses must accept the press explicitly (default QWidget ignores it, so the release never arrives) — see `mousePressEvent`
  - `input/searchinput` — QLineEdit with leading magnifier action (recolored on modeChanged) + `setClearButtonEnabled`, emits `searchRequested(text)` on return
  - `label/clickedlabel` — clickable QLabel that swallows the press event (for frameless title bars)
  - `label/marqueelabel` — auto-scrolling label when text overflows; hover pauses (Enter/Leave self-tracked — `underMouse()` is unreliable on some QPA platforms). `offset()` exposed for tests
  - `label/toastlabel` — auto-fading toast; static `ToastLabel::showText(text, parent)`; restyled from Theme inverse roles on every modeChanged
  - `label/transitionlabel` — cross-fade between pixmaps (first set shows directly); `progress()`/`isAnimating()` exposed for tests
  - `label/rotatelabel` — continuously rotating pixmap (record-player cover), optional circular clip; default stopped, `setRunning()` starts; WaitSpinner-style looped QVariantAnimation
  - `slider/clickedslider` — QSlider where a press anywhere jumps the handle there (then drags normally); extra `clicked()` signal; an empty-range press is explicitly accepted — QSlider's default ignore bubbles up to a frameless parent window and gets treated as "press on empty area" (window drag)
  - `slider/tipslider` — ClickedSlider + hover bubble (top-level Qt::ToolTip child, positioned in global coords — a child widget would be clipped); default formatter is `Duration::format(ms)`, `setFormatter()` replaces it; horizontal only
  - `progress/waitspinner` — indeterminate rotating-arc spinner (QVariantAnimation-driven `angle()`); default color follows Theme Primary unless `setColor()` was called (`m_customColor` flag)
  - `widget/animationstackedwidget` — QStackedWidget with slide transitions; requests during animation are ignored; destructor defensively detaches animation targets (destroying a running group mid-destruction otherwise crashes)
  - `widget/sidenav` — Ant-Menu-style vertical nav (selected item = primary tint + left indicator bar); click or Up/Down keys; plain-QWidget press-accept pattern again applies
  - `widget/coverflow` — coverflow strip with reflections (paints its own Background-role backdrop so the reflection fade gradient can reuse it); wheel/keys/drag; drag updates a live qreal position, release snaps with animation
  - `window/framelesswidget` — frameless window: client-area drag, 8-direction edge resize (5px default margin, `setResizeMargin()`), double-click maximize/restore. Prefers native `startSystemMove`/`startSystemResize` (Windows snap, Wayland), falls back to manual implementation when the platform doesn't take over.
  - `window/framelesshandler` — the event-filter object that implements the above behavior (extracted so `FramelessDialog` shares it; installed on the target window, never consumes the window's own events). `watch(panel)` additionally routes a covering panel's mouse events through the same logic with panel→window coordinate translation, consuming them so they never bubble to the window (double-handling); construct with `filterTarget=false` to get a watch-only carrier (TitleBar does this — do NOT let a second handler also filter the window, double-click maximize would toggle twice)
  - `window/framelessdialog` — QDialog with the same frameless behavior via FramelessHandler
  - `window/titlebar` — caption bar (title + min/max/close) for frameless windows; the bar itself is watched by a `FramelessHandler` (drag / edge resize / double-click maximize work on the bar; buttons are separate children that consume their own clicks — `WA_TransparentForMouseEvents` is NOT the mechanism, it silences children too and killed the caption buttons); CaptionButton lives in the .cpp with Q_OBJECT + `#include "titlebar.moc"` (AUTOMOC picks it up); max/restore glyph switched via eventFilter on WindowStateChange
  - `window/messagebox` — QMessageBox-style dialog on FramelessDialog + TitleBar + PushButton; icon glyph painted into a pixmap per invocation (not live-rethemed — dialogs are ephemeral); `question()` inserts the cancel button left of the ok button by object-name lookup
  - `window/notifywindow` — bottom-right desktop notification; `showMessage()` keeps a file-static active list for stacking (windows reposition when one dies); `WA_DeleteOnClose` + fade-out; labels are `WA_TransparentForMouseEvents` so clicks reach the window
  - `window/splashscreen` — heap-allocated splash (`WA_DeleteOnClose`), `finish(mainWindow)` fades windowOpacity then closes (opacity is a no-op on the offscreen QPA but the animation still completes — tests rely on that)

Each library is built under its plain name (`core`, `widgets`) but aliased as `canfan::core` / `canfan::widgets` — consumer projects link the `canfan::` aliases. When `common` is pulled in via `add_subdirectory()` from another project, the top-level Qt lookup is skipped (`PROJECT_IS_TOP_LEVEL` guard), so Qt is resolved by the parent project's own `find_package` (which also defines `QT_VERSION_MAJOR` for this scope); the parent must request at least `COMPONENTS Core Network Widgets` (core needs Network).

Include paths: `core/` and `widgets/` source roots are public, plus every category subdirectory (`core/base`, `core/application`, `widgets/{theme,button,input,label,progress,slider,widget,window}`) — so both `#include "clickedlabel.h"` and `#include "label/clickedlabel.h"` work (musicplayer relies on the short form). When adding a new category dir, expose it in `widgets/CMakeLists.txt` (`target_include_directories`) AND `common.pri` (`INCLUDEPATH`). New headers must be added to the `add_library()` source lists explicitly (AUTOMOC is on).

qmake is supported alongside CMake (deliberate keep decision, 2026-09-19 — do not propose dropping it): `common.pri` is the consumer entry point (compiles sources into the including project, TTKCommon-style), `common.pro` builds a static lib standalone, `examples/widgets_demo.pro` builds the gallery demo. When adding a file, update `common.pri` (HEADERS/SOURCES) as well as the CMake lists. Build qmake targets from a VS x64 prompt (`vcvars64.bat`) since nmake/cl need the MSVC env:

```sh
mkdir build-qmake/lib && cd build-qmake/lib && qmake ../../common.pro && nmake
mkdir build-qmake/demo && cd build-qmake/demo && qmake ../../examples/widgets_demo.pro && nmake
```

Code comments, README, and docs/ are in Chinese; keep that convention. Documentation layout: README.md is the front page (category-overview table, integration, build); docs/components.md is the full per-component reference (with usage snippets); docs/theming.md is the token/theme guide.

## Build

Qt 6.8.3 (msvc2022_64) is not on CMake's default search path — pass `CMAKE_PREFIX_PATH=D:/Qt/6.8.3/msvc2022_64`. With the Ninja generator, configure from a VS x64 developer prompt (MSVC `cl.exe` must be in PATH):

```sh
# configure (Ninja, existing build/ dir uses this)
cmake -B build -G Ninja -DCMAKE_PREFIX_PATH=D:/Qt/6.8.3/msvc2022_64 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# build + tests
cmake --build build
ctest --test-dir build
```

Qt 5.15.2 lives at `D:/Qt/5.15.2/msvc2019_64`; it builds with the same VS2022 toolchain (2019/2022 kits are ABI-compatible). Use a separate build dir, and put the Qt5 `bin` on PATH before `ctest` or the test exes fail with `0xc0000135` (missing Qt DLLs):

```sh
cmake -B build-qt5 -G Ninja -DCMAKE_PREFIX_PATH=D:/Qt/5.15.2/msvc2019_64
cmake --build build-qt5
ctest --test-dir build-qt5
```

Both configurations must stay green — build and test both before tagging a release. One toolchain pitfall: `build/` is pinned to the VS2022 cl on the D: drive; running its build under the VS18 vcvars env mixes a newer STL with the older cached compiler (STL1001). Always enter the environment matching the compiler a build dir was configured with.

CI runs the same matrix on GitHub Actions (`.github/workflows/ci.yml`: ubuntu/windows × Qt 5.15.2/6.8.3, install-qt-action + msvc-dev-cmd, build + ctest, offscreen on Linux) on every push to main, tags, and PRs.

`build-test/` is configured with the Visual Studio 17 2022 generator (open `build-test/common.sln`, or `cmake --build build-test`). Tests live in `tests/` (`tst_core`, `tst_widgets`, run on the offscreen QPA platform). Note: QTest console output is invisible when stdout is redirected from git-bash/cmd on this machine — run a test with `-o <file>,txt` to see results. Tests and examples only build when `common` is the top-level project. No lint/format tooling is configured. clangd is used (`.cache/clangd`; `compile_commands.json` lives in `build/`).

`examples/` is a TTKExample-style widget gallery: `GalleryWindow` (left category list + right `QStackedWidget`) with `addPage(name, page)`; one demo page class per widget under `examples/pages/`. To showcase a new widget: create `examples/pages/<widget>page.{h,cpp}`, register it in `examples/CMakeLists.txt` + `examples/widgets_demo.pro` (HEADERS/SOURCES), and add one `addPage(...)` line in `examples/main.cpp`.

## Adding a component

A component counts as complete only when all seven places are updated:

1. Source in the right category dir — `core/` must stay UI-free (`base/` for primitives, `application/` for app-level); controls go to `widgets/<category>/`, new category dirs allowed
2. One name added to the `canfan_register_widget(<category> ...)` line in `widgets/CMakeLists.txt` (the helper in `widgets/register.cmake` expands sources and the category include dir automatically — new category = new line)
3. `common.pri` HEADERS/SOURCES
4. A test case in `tests/tst_core.cpp` / `tst_widgets.cpp`
5. A gallery demo page: `examples/pages/<name>page.{h,cpp}` + registration in `examples/CMakeLists.txt`, `examples/widgets_demo.pro`, and one `addPage(...)` line in `examples/main.cpp`
6. An entry in docs/components.md under the component's category (plus a usage snippet when the API is non-obvious), and the component name added to that category's row in README's overview table
7. Version bump in the root CMakeLists + git tag, per Versioning below

`common` grows on demand — add a component when a consumer project actually needs it; do not mirror TTKCommon's full catalog.

## Versioning

Consumers pin this library via CMake `FetchContent` with a `vX.Y.Z` git tag (see musicplayer's CMakeLists.txt). After any interface or feature change: commit, `git tag vX.(Y+1).0`, `git push --tags`. Never move an already-consumed tag; bump to a new one instead. Keep `project(... VERSION x.y.z)` in the root CMakeLists in sync with the tag.
