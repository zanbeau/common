# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

Shared base library (`common`) for personal Qt projects. Two static libraries, layered strictly:

- `core/` — Qt6::Core only, **no UI dependencies** (e.g. `base/singleton.h`, CRTP singleton)
- `widgets/` — Qt6::Widgets controls, depends on `canfan::core` (e.g. `FramelessWidget`: frameless + client-area drag + edge drag-resize, 6px margin)

Each library is built under its plain name (`core`, `widgets`) but aliased as `canfan::core` / `canfan::widgets` — consumer projects link the `canfan::` aliases. When `common` is pulled in via `add_subdirectory()` from another project, the top-level `find_package(Qt6)` is skipped (`PROJECT_IS_TOP_LEVEL` guard), so Qt is resolved by the parent project.

Include paths are the library source roots themselves (`core/`, `widgets/`), not per-subdirectory — headers are included as `base/singleton.h` or `framelesswidget.h`. New headers must be added to the `add_library()` source lists explicitly (AUTOMOC is on).

Code comments and README are in Chinese; keep that convention.

## Build

Qt 6.8.3 (msvc2022_64) is not on CMake's default search path — pass `CMAKE_PREFIX_PATH=D:/Qt/6.8.3/msvc2022_64`. With the Ninja generator, configure from a VS x64 developer prompt (MSVC `cl.exe` must be in PATH):

```sh
# configure (Ninja, existing build/ dir uses this)
cmake -B build -G Ninja -DCMAKE_PREFIX_PATH=D:/Qt/6.8.3/msvc2022_64 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# build
cmake --build build
```

`build-test/` is configured with the Visual Studio 17 2022 generator (open `build-test/common.sln`, or `cmake --build build-test`). There is no test suite; no lint/format tooling is configured. clangd is used (`.cache/clangd`; `compile_commands.json` lives in `build/`).

## Versioning

Consumers pin this library via CMake `FetchContent` with a `vX.Y.Z` git tag (see musicplayer's CMakeLists.txt). After any interface or feature change: commit, `git tag vX.(Y+1).0`, `git push --tags`. Never move an already-consumed tag; bump to a new one instead.
