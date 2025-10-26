cmake_minimum_required(VERSION 3.16)
project(SystemManager)


file(GLOB QT_PATHS "/usr/local/Qt-*")
if(QT_PATHS)
    list(SORT QT_PATHS)
    list(REVERSE QT_PATHS)
    list(GET QT_PATHS 0 LATEST_QT_PATH)
    list(APPEND CMAKE_PREFIX_PATH "${LATEST_QT_PATH}")
    message(STATUS "Using custom Qt path: ${LATEST_QT_PATH}")
else()
    message(STATUS "Using system Qt installation")
endif()
list(APPEND CMAKE_PREFIX_PATH "/usr")


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_AUTOGEN_BUILD_DIR ${CMAKE_BINARY_DIR}/autogen)
set(CMAKE_RCC_BUILD_DIR ${CMAKE_BINARY_DIR}/rcc)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Charts
    Multimedia
)

find_package(Qt6 COMPONENTS WaylandClient)

set(CMAKE_CXX_STANDARD 23)
if(TARGET Qt6::WaylandClient)
    add_compile_definitions(QT_WAYLAND)
    message(STATUS "Wayland support enabled")
else()
    message(WARNING "Wayland support not found - building with X11 only")
endif()

set(APP_SOURCES
    src/addautostartdialog.cpp
    src/logger.cpp
    src/mainwindow.cpp
    src/resource_monitor.cpp
    src/services.cpp
    src/temp_files.cpp
    src/autostart.cpp
    src/config_manager.cpp
    src/task_manager.cpp
    src/start_new_process_dialog.cpp
)

set(HEADERS
    include/addautostartdialog.hpp
    include/mainwindow.h
    include/resource_monitor.hpp
    include/services.hpp
    include/temp_files.hpp
    include/autostart.hpp
    include/logger.hpp
    include/config_manager.hpp
    include/task_manager.hpp
    include/start_new_process_dialog.hpp
)

set(FORMS
    mainwindow.ui
)

set(RESOURCES
    resources/icon.qrc
    resources/themes.qrc
    resources/haha.qrc
)

add_executable(${PROJECT_NAME}
    src/main.cpp
    ${APP_SOURCES}
    ${HEADERS}
    ${FORMS}
    ${RESOURCES}


)

target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Charts
)

if(TARGET Qt6::WaylandClient)
    target_link_libraries(${PROJECT_NAME} Qt6::WaylandClient)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(${PROJECT_NAME} systemd)
endif()


install(TARGETS ${PROJECT_NAME} DESTINATION bin)






install(FILES ${QM_FILES}
        DESTINATION share/${PROJECT_NAME}/translations
)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)





configure_file(
    packaging/SystemManager.desktop
    ${CMAKE_CURRENT_BINARY_DIR}/SystemManager.desktop
    @ONLY
)

# Install the modified desktop file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/SystemManager.desktop
    DESTINATION share/applications
)

# Packaging configuration
set(CPACK_PACKAGE_NAME "System Manager")
set(CPACK_PACKAGE_VENDOR "apelsin")
set(CPACK_PACKAGE_VERSION "1.0.2")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "System Management Tool")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")

# DEB specific settings
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "apelsin <ggwp.gg6578@gmail.com>")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt6core6, libqt6gui6, libqt6widgets6, libqt6charts6, policykit-1")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")

# RPM specific settings
set(CPACK_RPM_PACKAGE_REQUIRES "qt6-qtbase >= 6.2, qt6-qtcharts >= 6.2, polkit")
set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/usr/bin")
set(CPACK_RPM_SPEC_MORE_DEFINES "%define _binary_payload w9.gzdio")
set(CPACK_RPM_USER_FILELIST
    "%attr(755,root,root) /usr/bin/systemmanager-start"
)

include(CPack)
