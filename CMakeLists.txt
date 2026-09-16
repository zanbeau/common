cmake_minimum_required(VERSION 3.21)
project(common VERSION 0.1.0 LANGUAGE CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# 单独打开构建时自己找 Qt

if(PROJECT_IS_TOP_LEVEL)
    find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
endif()

add_subdirectory(core)
add_subdirectory(widgets)

common\core\CMakeLists.txt
add_library(core STATIC
    base/singleton.h
)
add_library(canfan::core ALIAS core)
target_link_libraries(core PUBLIC Qt6::Core)
target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
