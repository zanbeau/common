# common 库的 qmake 接入文件
#
# 用法:在业务工程(.pro)里加入一行
#   include(<common 路径>/common.pri)
# common 的源码会直接编入使用方工程(与 TTKCommon 相同的合入方式),
# 头文件目录同时加入 INCLUDEPATH,业务代码直接 #include "xxx.h" 即可。

INCLUDEPATH += $$PWD/core \
               $$PWD/core/base \
               $$PWD/widgets/frameless \
               $$PWD/widgets/controls

QT      += widgets
CONFIG  += c++17

HEADERS += $$PWD/core/base/singleton.h \
           $$PWD/widgets/frameless/framelesswidget.h \
           $$PWD/widgets/controls/clickedlabel.h \
           $$PWD/widgets/controls/toastlabel.h

SOURCES += $$PWD/widgets/frameless/framelesswidget.cpp \
           $$PWD/widgets/controls/clickedlabel.cpp \
           $$PWD/widgets/controls/toastlabel.cpp

msvc {
    QMAKE_CXXFLAGS += /utf-8
}
