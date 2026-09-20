# 组件注册:一行一个类别;新增组件 = 在对应行加一个名字
# (源文件与类别 include 目录由 register.cmake 自动展开)
include(register.cmake)

canfan_register_widget(button pushbutton toggleswitch iconbutton)
canfan_register_widget(input searchinput)
canfan_register_widget(label clickedlabel marqueelabel toastlabel
                             transitionlabel rotatelabel)
canfan_register_widget(progress waitspinner)
canfan_register_widget(slider clickedslider tipslider)
canfan_register_widget(theme tokens theme)   # tokens 为纯头文件组件
canfan_register_widget(widget animationstackedwidget sidenav coverflow)
canfan_register_widget(window framelesswidget framelesshandler framelessshadow
                             framelessdialog titlebar messagebox notifywindow
                             splashscreen)

add_library(widgets STATIC ${CANFAN_WIDGET_SOURCES})
add_library(canfan::widgets ALIAS widgets)
target_link_libraries(widgets PUBLIC Qt${QT_VERSION_MAJOR}::Widgets canfan::core)
# 目录名 = 控件类别,每个类别目录都暴露:
# #include "clickedlabel.h" 与 #include "label/clickedlabel.h" 两种写法都可用
target_include_directories(widgets PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CANFAN_WIDGET_CATEGORY_DIRS}
)
