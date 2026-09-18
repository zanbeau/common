add_library(core STATIC
    app/singleinstance.h
    app/singleinstance.cpp
    base/logger.h
    base/logger.cpp
    base/singleton.h
)
add_library(canfan::core ALIAS core)
target_link_libraries(core PUBLIC Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Network)
target_include_directories(core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/base
    ${CMAKE_CURRENT_SOURCE_DIR}/app
)
