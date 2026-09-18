add_executable(widgets_demo WIN32
    main.cpp
    gallerywindow.h
    gallerywindow.cpp
    pages/framelessdemowindow.h
    pages/framelessdemowindow.cpp
    pages/framelesspage.h
    pages/framelesspage.cpp
    pages/clickedlabelpage.h
    pages/clickedlabelpage.cpp
    pages/clickedsliderpage.h
    pages/clickedsliderpage.cpp
    pages/marqueepage.h
    pages/marqueepage.cpp
    pages/waitspinnerpage.h
    pages/waitspinnerpage.cpp
    pages/animationstackedpage.h
    pages/animationstackedpage.cpp
    pages/toastpage.h
    pages/toastpage.cpp
)
target_link_libraries(widgets_demo PRIVATE canfan::widgets)
