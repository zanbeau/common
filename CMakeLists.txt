cmake_minimum_required(VERSION 3.21)
project(common VERSION 0.13.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

# 禁用 Qt 5.15 之前已弃用的 API:保证同一份源码在 Qt 5.15 / Qt 6 下都干净编译
add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x050F00)

# 单独打开构建时自己找 Qt(优先 Qt6,可通过 CMAKE_PREFIX_PATH 指到 5.15)
if(PROJECT_IS_TOP_LEVEL)
    find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Network Widgets Test)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network Widgets Test)
endif()

add_subdirectory(core)
add_subdirectory(widgets)

# 示例与测试只在单独构建本库时编译
if(PROJECT_IS_TOP_LEVEL)
    enable_testing()
    add_subdirectory(examples)
    add_subdirectory(tests)
endif()
