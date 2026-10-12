find_package(Qt6 REQUIRED COMPONENTS Test)

qt_add_executable(tst_engine tst_engine.cpp)
target_link_libraries(tst_engine PRIVATE ImgEngine Qt6::Test)
target_compile_definitions(tst_engine PRIVATE
    TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
    ASSETS_DIR="${PROJECT_SOURCE_DIR}/assets")
set_target_properties(tst_engine PROPERTIES MACOSX_BUNDLE OFF WIN32_EXECUTABLE OFF)

add_test(NAME tst_engine COMMAND tst_engine)
# Text rendering needs a QGuiApplication but no display.
set_tests_properties(tst_engine PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
