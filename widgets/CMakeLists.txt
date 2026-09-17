add_library(widgets STATIC
    controls/clickedlabel.h
    controls/clickedlabel.cpp
    controls/toastlabel.h
    controls/toastlabel.cpp
    frameless/framelesswidget.h
    frameless/framelesswidget.cpp
)
add_library(canfan::widgets ALIAS widgets)
target_link_libraries(widgets PUBLIC Qt6::Widgets canfan::core)
# 同时暴露子目录,#include "framelesswidget.h" 与 #include "frameless/framelesswidget.h" 两种写法都可用
target_include_directories(widgets PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/controls
    ${CMAKE_CURRENT_SOURCE_DIR}/frameless
)
