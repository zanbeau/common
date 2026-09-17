cmake_minimum_required(VERSION 3.21)
project(common VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# 单独打开构建时自己找 Qt

if(PROJECT_IS_TOP_LEVEL)
    find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
endif()

add_subdirectory(core)
add_subdirectory(widgets)
