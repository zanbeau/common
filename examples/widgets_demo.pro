# qmake 构建示例(控件浏览器):qmake widgets_demo.pro && mingw32-make / nmake
TEMPLATE = app
TARGET   = widgets_demo
include($$PWD/../common.pri)

INCLUDEPATH += $$PWD

HEADERS += gallerywindow.h \
           pages/framelessdemowindow.h \
           pages/framelesspage.h \
           pages/framelessdialogpage.h \
           pages/titlebarpage.h \
           pages/messageboxpage.h \
           pages/clickedlabelpage.h \
           pages/clickedsliderpage.h \
           pages/marqueepage.h \
           pages/waitspinnerpage.h \
           pages/animationstackedpage.h \
           pages/sidenavpage.h \
           pages/coverflowpage.h \
           pages/playlisttablepage.h \
           pages/lyricsviewpage.h \
           pages/toastpage.h \
           pages/themepage.h \
           pages/pushbuttonpage.h \
           pages/iconbuttonpage.h \
           pages/toggleswitchpage.h \
           pages/searchinputpage.h \
           pages/tipsliderpage.h \
           pages/transitionlabelpage.h \
           pages/rotatelabelpage.h \
           pages/notifywindowpage.h \
           pages/splashscreenpage.h

SOURCES += main.cpp \
           gallerywindow.cpp \
           pages/framelessdemowindow.cpp \
           pages/framelesspage.cpp \
           pages/framelessdialogpage.cpp \
           pages/titlebarpage.cpp \
           pages/messageboxpage.cpp \
           pages/clickedlabelpage.cpp \
           pages/clickedsliderpage.cpp \
           pages/marqueepage.cpp \
           pages/waitspinnerpage.cpp \
           pages/animationstackedpage.cpp \
           pages/sidenavpage.cpp \
           pages/coverflowpage.cpp \
           pages/playlisttablepage.cpp \
           pages/lyricsviewpage.cpp \
           pages/toastpage.cpp \
           pages/themepage.cpp \
           pages/pushbuttonpage.cpp \
           pages/iconbuttonpage.cpp \
           pages/toggleswitchpage.cpp \
           pages/searchinputpage.cpp \
           pages/tipsliderpage.cpp \
           pages/transitionlabelpage.cpp \
           pages/rotatelabelpage.cpp \
           pages/notifywindowpage.cpp \
           pages/splashscreenpage.cpp
