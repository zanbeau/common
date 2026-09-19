# 组件注册:canfan_register_widget(<类别> <组件名>...)
#  - 组件名对应 <类别>/<组件名>.h/.cpp,纯头文件组件(无 .cpp)自动跳过源文件
#  - 类别目录自动加入公开 include 路径(目录名 = 类别的仓库约定不变)
# 在 widgets/CMakeLists.txt 顶部 include() 本文件后使用;
# 累积结果经 CANFAN_WIDGET_SOURCES / CANFAN_WIDGET_CATEGORY_DIRS 两个父作用域变量带回
function(canfan_register_widget category)
    foreach(name IN LISTS ARGN)
        list(APPEND CANFAN_WIDGET_SOURCES ${category}/${name}.h)
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${category}/${name}.cpp")
            list(APPEND CANFAN_WIDGET_SOURCES ${category}/${name}.cpp)
        endif()
        list(APPEND CANFAN_WIDGET_CATEGORY_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/${category})
    endforeach()
    list(REMOVE_DUPLICATES CANFAN_WIDGET_CATEGORY_DIRS)
    set(CANFAN_WIDGET_SOURCES ${CANFAN_WIDGET_SOURCES} PARENT_SCOPE)
    set(CANFAN_WIDGET_CATEGORY_DIRS ${CANFAN_WIDGET_CATEGORY_DIRS} PARENT_SCOPE)
endfunction()
