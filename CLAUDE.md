# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

Shared base library (`common`) for personal Qt projects. Two static libraries, layered strictly:

- `core/` — Qt6::Core + Qt6::Network (QLocalServer/QLocalSocket), **no UI dependencies**
  - `base/singleton.h` — CRTP singleton
  - `base/logger` — stream logger (`Log::info() << ...`); `Log::setFile()` installs a message handler mirroring ALL Qt messages to a file
  - `app/singleinstance` — composition-based single-instance guard (no QApplication subclass, unlike TTK's three-class approach). Secondary instances `sendMessage()` (newline-framed, blocks until the primary acks — the ack is required: without it a secondary exiting right after write can lose the message). Consumer apps must keep the primary's event loop responsive.
- `widgets/` — Qt6::Widgets controls, depends on `canfan::core`
  - `frameless/framelesswidget` — frameless window: client-area drag, 8-direction edge resize (5px default margin, `setResizeMargin()`), double-click maximize/restore. Prefers native `startSystemMove`/`startSystemResize` (Windows snap, Wayland), falls back to manual implementation when the platform doesn't take over.
  - `controls/clickedlabel` — clickable QLabel that swallows the press event (for frameless title bars)
  - `controls/toastlabel` — auto-fading toast; static `ToastLabel::showText(text, parent)`

Each library is built under its plain name (`core`, `widgets`) but aliased as `canfan::core` / `canfan::widgets` — consumer projects link the `canfan::` aliases. When `common` is pulled in via `add_subdirectory()` from another project, the top-level `find_package(Qt6)` is skipped (`PROJECT_IS_TOP_LEVEL` guard), so Qt is resolved by the parent project; the parent must request at least `COMPONENTS Core Network Widgets` (core needs Network).

Include paths: `core/` and `widgets/` source roots are public, plus the `widgets/frameless/` and `widgets/controls/` subdirectories — so both `#include "framelesswidget.h"` and `#include "frameless/framelesswidget.h"` work (musicplayer relies on the short form). New headers must be added to the `add_library()` source lists explicitly (AUTOMOC is on).

qmake is supported alongside CMake: `common.pri` is the consumer entry point (compiles sources into the including project, TTKCommon-style), `common.pro` builds a static lib standalone, `examples/widgets_demo.pro` builds the gallery demo. When adding a file, update `common.pri` (HEADERS/SOURCES) as well as the CMake lists. Build qmake targets from a VS x64 prompt (`vcvars64.bat`) since nmake/cl need the MSVC env:

```sh
mkdir build-qmake/lib && cd build-qmake/lib && qmake ../../common.pro && nmake
mkdir build-qmake/demo && cd build-qmake/demo && qmake ../../examples/widgets_demo.pro && nmake
```

Code comments and README are in Chinese; keep that convention.

## Build

Qt 6.8.3 (msvc2022_64) is not on CMake's default search path — pass `CMAKE_PREFIX_PATH=D:/Qt/6.8.3/msvc2022_64`. With the Ninja generator, configure from a VS x64 developer prompt (MSVC `cl.exe` must be in PATH):

```sh
# configure (Ninja, existing build/ dir uses this)
cmake -B build -G Ninja -DCMAKE_PREFIX_PATH=D:/Qt/6.8.3/msvc2022_64 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# build + tests
cmake --build build
ctest --test-dir build
```

`build-test/` is configured with the Visual Studio 17 2022 generator (open `build-test/common.sln`, or `cmake --build build-test`). Tests live in `tests/` (`tst_core`, `tst_widgets`, run on the offscreen QPA platform). Note: QTest console output is invisible when stdout is redirected from git-bash/cmd on this machine — run a test with `-o <file>,txt` to see results. Tests and examples only build when `common` is the top-level project. No lint/format tooling is configured. clangd is used (`.cache/clangd`; `compile_commands.json` lives in `build/`).

`examples/` is a TTKExample-style widget gallery: `GalleryWindow` (left category list + right `QStackedWidget`) with `addPage(name, page)`; one demo page class per widget under `examples/pages/`. To showcase a new widget: create `examples/pages/<widget>page.{h,cpp}`, register it in `examples/CMakeLists.txt` + `examples/widgets_demo.pro` (HEADERS/SOURCES), and add one `addPage(...)` line in `examples/main.cpp`.

## Versioning

Consumers pin this library via CMake `FetchContent` with a `vX.Y.Z` git tag (see musicplayer's CMakeLists.txt). After any interface or feature change: commit, `git tag vX.(Y+1).0`, `git push --tags`. Never move an already-consumed tag; bump to a new one instead. Keep `project(... VERSION x.y.z)` in the root CMakeLists in sync with the tag.
