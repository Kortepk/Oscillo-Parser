cmake_minimum_required(VERSION 3.5)

project(Oscillo-Parser VERSION 0.1 LANGUAGES CXX)

set(CMAKE_GENERATOR "Ninja")

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
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Oscillo-Parser
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET Oscillo-Parser APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(Oscillo-Parser SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(Oscillo-Parser
            ${PROJECT_SOURCES}
            settingsdialog.h settingsdialog.cpp settingsdialog.ui
            controlpanel.h controlpanel.cpp controlpanel.ui
            oscillo_channel.h oscillo_channel.cpp

        )
    endif()
endif()

target_link_libraries(Oscillo-Parser PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.Oscillo-Parser)
endif()
set_target_properties(Oscillo-Parser PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS Oscillo-Parser
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Oscillo-Parser)
endif()

find_package(Qt5  REQUIRED COMPONENTS SerialPort)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::SerialPort)

set(Qt5Charts_DIR "D:/Programs/QT-5/5.15.0/mingw81_64/lib/cmake/Qt5Charts/Qt5ChartsConfig.cmake")
find_package(Qt5 REQUIRED COMPONENTS Charts)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Charts)
