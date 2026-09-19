add_library(widgets STATIC
    button/pushbutton.h
    button/pushbutton.cpp
    button/toggleswitch.h
    button/toggleswitch.cpp
    input/searchinput.h
    input/searchinput.cpp
    label/clickedlabel.h
    label/clickedlabel.cpp
    label/marqueelabel.h
    label/marqueelabel.cpp
    label/toastlabel.h
    label/toastlabel.cpp
    label/transitionlabel.h
    label/transitionlabel.cpp
    label/rotatelabel.h
    label/rotatelabel.cpp
    progress/waitspinner.h
    progress/waitspinner.cpp
    slider/clickedslider.h
    slider/clickedslider.cpp
    slider/tipslider.h
    slider/tipslider.cpp
    theme/tokens.h
    theme/theme.h
    theme/theme.cpp
    widget/animationstackedwidget.h
    widget/animationstackedwidget.cpp
    window/framelesswidget.h
    window/framelesswidget.cpp
    window/framelesshandler.h
    window/framelesshandler.cpp
    window/framelessdialog.h
    window/framelessdialog.cpp
    window/messagebox.h
    window/messagebox.cpp
    window/notifywindow.h
    window/notifywindow.cpp
    window/splashscreen.h
    window/splashscreen.cpp
    window/titlebar.h
    window/titlebar.cpp
)
add_library(canfan::widgets ALIAS widgets)
target_link_libraries(widgets PUBLIC Qt${QT_VERSION_MAJOR}::Widgets canfan::core)
# 目录名 = 控件类别(button/input/label/progress/slider/theme/widget/window),每个类别目录都暴露:
# #include "clickedlabel.h" 与 #include "label/clickedlabel.h" 两种写法都可用
target_include_directories(widgets PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/button
    ${CMAKE_CURRENT_SOURCE_DIR}/input
    ${CMAKE_CURRENT_SOURCE_DIR}/label
    ${CMAKE_CURRENT_SOURCE_DIR}/progress
    ${CMAKE_CURRENT_SOURCE_DIR}/slider
    ${CMAKE_CURRENT_SOURCE_DIR}/theme
    ${CMAKE_CURRENT_SOURCE_DIR}/widget
    ${CMAKE_CURRENT_SOURCE_DIR}/window
)
