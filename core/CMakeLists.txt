add_library(core STATIC
    base/singleton.h
)
add_library(canfan::core ALIAS core)
target_link_libraries(core PUBLIC Qt6::Core)
target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
