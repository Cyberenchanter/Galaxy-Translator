cmake_minimum_required(VERSION 3.5)

project(Galaxy-Translator VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        iodialog.h
        iodialog.cpp
        iodialog.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Galaxy-Translator
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET Galaxy-Translator APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(Galaxy-Translator SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(Galaxy-Translator
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(Galaxy-Translator PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(Galaxy-Translator PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS Galaxy-Translator
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

set(Galaxy-Translator_resource_files
    "images/copy.png"
    "images/cut.png"
    "images/new.png"
    "images/open.png"
    "images/paste.png"
    "images/save.png"
)
qt_add_resources(Galaxy-Translator "Galaxy-Translator"
    PREFIX
        "/"
    FILES
        ${Galaxy-Translator_resource_files}
)


if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Galaxy-Translator)
endif()
