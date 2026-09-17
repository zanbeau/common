# qmake 独立构建 common(静态库)
# 业务工程更推荐直接 include(common.pri) 把源码编入工程
TEMPLATE = lib
TARGET   = common
CONFIG  += staticlib
include(common.pri)
