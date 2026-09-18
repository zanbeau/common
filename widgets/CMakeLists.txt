add_library(widgets STATIC
    label/clickedlabel.h
    label/clickedlabel.cpp
    label/marqueelabel.h
    label/marqueelabel.cpp
    label/toastlabel.h
    label/toastlabel.cpp
    progress/waitspinner.h
    progress/waitspinner.cpp
    slider/clickedslider.h
    slider/clickedslider.cpp
    widget/animationstackedwidget.h
    widget/animationstackedwidget.cpp
    window/framelesswidget.h
    window/framelesswidget.cpp
)
add_library(canfan::widgets ALIAS widgets)
target_link_libraries(widgets PUBLIC Qt${QT_VERSION_MAJOR}::Widgets canfan::core)
# 目录名 = 控件类别(label/slider/progress/widget/window),每个类别目录都暴露:
# #include "clickedlabel.h" 与 #include "label/clickedlabel.h" 两种写法都可用
target_include_directories(widgets PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/label
    ${CMAKE_CURRENT_SOURCE_DIR}/progress
    ${CMAKE_CURRENT_SOURCE_DIR}/slider
    ${CMAKE_CURRENT_SOURCE_DIR}/widget
    ${CMAKE_CURRENT_SOURCE_DIR}/window
)
