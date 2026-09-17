# qmake 构建示例:qmake widgets_demo.pro && mingw32-make / nmake
TEMPLATE = app
TARGET   = widgets_demo
include($$PWD/../common.pri)
SOURCES += widgets_demo.cpp
