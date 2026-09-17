add_executable(tst_widgets tst_widgets.cpp)
target_link_libraries(tst_widgets PRIVATE canfan::widgets Qt6::Test)

add_test(NAME tst_widgets COMMAND tst_widgets)
set_tests_properties(tst_widgets PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
