cmake_minimum_required(VERSION 3.21)
# The release pipeline passes the semantic-release version.
set(APP_VERSION "0.0.0" CACHE STRING "Version embedded in the app bundle and executable")
project(ImgSettings VERSION ${APP_VERSION} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.9 REQUIRED COMPONENTS Gui Qml Quick QuickControls2 ShaderTools)
qt_standard_project_setup(REQUIRES 6.9)

set(APP_ICON ${CMAKE_CURRENT_SOURCE_DIR}/assets/AppIcon.icns)
set_source_files_properties(${APP_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

qt_add_executable(ImgSettings MACOSX_BUNDLE WIN32
    src/main.cpp
    src/exif.h src/exif.cpp
    src/watermark.h src/watermark.cpp
    src/processor.h src/processor.cpp
    ${APP_ICON}
)

if(WIN32)
    target_sources(ImgSettings PRIVATE assets/AppIcon.rc)
endif()
target_compile_definitions(ImgSettings PRIVATE APP_VERSION="${PROJECT_VERSION}")

set_source_files_properties(qml/Theme.qml qml/Icons.qml qml/Fluent.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(ImgSettings
    URI ImgSettings
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/MacShell.qml
        qml/WindowsShell.qml
        qml/Theme.qml
        qml/Fluent.qml
        qml/Icons.qml
        qml/Icon.qml
        qml/Backdrop.qml
        qml/GlassPanel.qml
        qml/GlassButton.qml
        qml/GlassSwitch.qml
        qml/GlassSlider.qml
        qml/Toolbar.qml
        qml/DropContent.qml
        qml/ResultsContent.qml
        qml/ResultTile.qml
        qml/Toast.qml
        qml/FluentDropContent.qml
        qml/FluentResults.qml
        qml/FluentResultTile.qml
        qml/InfoBar.qml
)

qt_add_shaders(ImgSettings "shaders"
    PREFIX "/"
    FILES shaders/glass.frag
)

qt_add_resources(ImgSettings "assets"
    PREFIX "/"
    FILES assets/AppIcon.png assets/Go-Regular.ttf
)

target_include_directories(ImgSettings PRIVATE src)

# JPEG XL input via libjxl (optional: without it, .jxl files are simply not offered).
option(JXL_STATIC "Link libjxl and its dependencies statically" OFF)
option(REQUIRE_JXL "Fail the configure step if libjxl is missing" OFF)
find_package(PkgConfig)
if(PkgConfig_FOUND)
    pkg_check_modules(JXL IMPORTED_TARGET libjxl libjxl_cms)
endif()
if(JXL_FOUND)
    target_sources(ImgSettings PRIVATE src/jxl.h src/jxl.cpp)
    if(JXL_STATIC)
        # Static libjxl (the release builds use vcpkg): link its private deps too.
        target_include_directories(ImgSettings PRIVATE ${JXL_STATIC_INCLUDE_DIRS})
        # Resolve each library to a file: on MSVC libjxl.pc names "jxl-static"
        # while the installed library is jxl.lib.
        foreach(lib IN LISTS JXL_STATIC_LIBRARIES)
            string(REGEX REPLACE "-static$" "" plain "${lib}")
            find_library(JXL_DEP_${lib} NAMES ${lib} ${plain}
                         HINTS ${JXL_STATIC_LIBRARY_DIRS} NO_DEFAULT_PATH)
            if(JXL_DEP_${lib})
                target_link_libraries(ImgSettings PRIVATE ${JXL_DEP_${lib}})
            elseif(NOT MSVC)
                target_link_libraries(ImgSettings PRIVATE ${lib}) # system libs (c++, m)
            endif()
        endforeach()
        target_compile_definitions(ImgSettings PRIVATE JXL_STATIC_DEFINE JXL_CMS_STATIC_DEFINE)
    else()
        target_link_libraries(ImgSettings PRIVATE PkgConfig::JXL)
    endif()
    target_compile_definitions(ImgSettings PRIVATE HAVE_JXL)
elseif(REQUIRE_JXL)
    message(FATAL_ERROR "libjxl not found")
else()
    message(WARNING "libjxl not found: JPEG XL (.jxl) input is disabled")
endif()

target_link_libraries(ImgSettings PRIVATE Qt6::Gui Qt6::Qml Qt6::Quick Qt6::QuickControls2)

set_target_properties(ImgSettings PROPERTIES
    OUTPUT_NAME img-settings
    MACOSX_BUNDLE_BUNDLE_NAME img-settings
    MACOSX_BUNDLE_GUI_IDENTIFIER io.img-settings.app
    MACOSX_BUNDLE_ICON_FILE AppIcon.icns
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION})

install(TARGETS ImgSettings BUNDLE DESTINATION . RUNTIME DESTINATION bin)
