add_library(widgets STATIC
    framelesswidget.h
    framelesswidget.cpp
)
add_library(canfan::widgets ALIAS widgets)
target_link_libraries(widgets PUBLIC Qt6::Widgets canfan::core)
target_include_directories(widgets PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
