# common 库的 qmake 接入文件
#
# 用法:在业务工程(.pro)里加入一行
#   include(<common 路径>/common.pri)
# common 的源码会直接编入使用方工程(与 TTKCommon 相同的合入方式),
# 头文件目录同时加入 INCLUDEPATH,业务代码直接 #include "xxx.h" 即可。

# 目录名 = 控件类别(core: base/application;widgets: label/window/...)
INCLUDEPATH += $$PWD/core \
               $$PWD/core/base \
               $$PWD/core/application \
               $$PWD/widgets/label \
               $$PWD/widgets/window

QT      += widgets network
CONFIG  += c++17

# 禁用 Qt 5.15 之前已弃用的 API,与 CMake 侧 QT_DISABLE_DEPRECATED_BEFORE 保持一致
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x050F00

HEADERS += $$PWD/core/base/singleton.h \
           $$PWD/core/base/logger.h \
           $$PWD/core/application/singleinstance.h \
           $$PWD/widgets/window/framelesswidget.h \
           $$PWD/widgets/label/clickedlabel.h \
           $$PWD/widgets/label/toastlabel.h

SOURCES += $$PWD/core/base/logger.cpp \
           $$PWD/core/application/singleinstance.cpp \
           $$PWD/widgets/window/framelesswidget.cpp \
           $$PWD/widgets/label/clickedlabel.cpp \
           $$PWD/widgets/label/toastlabel.cpp

msvc {
    QMAKE_CXXFLAGS += /utf-8
}
