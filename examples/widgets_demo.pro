# qmake 构建示例(控件浏览器):qmake widgets_demo.pro && mingw32-make / nmake
TEMPLATE = app
TARGET   = widgets_demo
include($$PWD/../common.pri)

INCLUDEPATH += $$PWD

HEADERS += gallerywindow.h \
           pages/framelessdemowindow.h \
           pages/framelesspage.h \
           pages/clickedlabelpage.h \
           pages/toastpage.h

SOURCES += main.cpp \
           gallerywindow.cpp \
           pages/framelessdemowindow.cpp \
           pages/framelesspage.cpp \
           pages/clickedlabelpage.cpp \
           pages/toastpage.cpp
