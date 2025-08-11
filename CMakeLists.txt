cmake_minimum_required(VERSION 3.16)
project(SystemManager)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_AUTOGEN_BUILD_DIR ${CMAKE_BINARY_DIR}/autogen)
set(CMAKE_RCC_BUILD_DIR ${CMAKE_BINARY_DIR}/rcc)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Charts LinguistTools)
set(CMAKE_CXX_STANDARD 23)

set(APP_SOURCES
    src/addautostartdialog.cpp
    src/logger.cpp
    src/mainwindow.cpp
    src/resource_monitor.cpp
    src/services.cpp
    src/temp_files.cpp
    src/autostart.cpp
)

set(HEADERS
    include/addautostartdialog.hpp
    include/mainwindow.h
    include/resource_monitor.hpp
    include/services.hpp
    include/temp_files.hpp
    include/autostart.hpp
    include/logger.hpp
)

set(FORMS
    mainwindow.ui
)

set(RESOURCES
    resources/translations.qrc
    resources/icon.qrc
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

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(${PROJECT_NAME} systemd)
endif()

qt6_add_translation(QM_FILES
    translations/qtguiinterface_en_US.ts
    translations/qtguiinterface_ru_RU.ts
    translations/qtguiinterface_pl_PL.ts
)
add_custom_target(translations ALL DEPENDS ${QM_FILES})

# Установка основных файлов
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

# Установка иконки (несколько размеров для поддержки разных окружений)
install(FILES packaging/SystemManager.ico
        DESTINATION share/icons/hicolor/256x256/apps
        RENAME SystemManager.png
)

install(FILES packaging/SystemManager.ico
        DESTINATION share/icons/hicolor/48x48/apps
        RENAME SystemManager.png
)

install(FILES packaging/SystemManager.ico
        DESTINATION share/icons/hicolor/32x32/apps
        RENAME SystemManager.png
)

# Установка .desktop файла
install(FILES packaging/SystemManager.desktop
        DESTINATION share/applications
)

# Установка переводов
install(FILES ${QM_FILES}
        DESTINATION share/${PROJECT_NAME}/translations
)

# Package configuration
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VENDOR "apelsin")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "System Management Tool")
set(CPACK_PACKAGE_CONTACT "ggwp.gg6578@gmail.com")
set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/packaging/SystemManager.ico")

# RPM specific settings
set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_RPM_PACKAGE_REQUIRES "qt6-qtbase >= 6.2.0, systemd, hicolor-icon-theme")

# DEB specific settings
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt6core6 (>= 6.2.0), libqt6gui6 (>= 6.2.0), libqt6widgets6 (>= 6.2.0), libqt6charts6 (>= 6.2.0), systemd, hicolor-icon-theme")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")

# Tarball (TGZ) configuration
set(CPACK_GENERATOR "TGZ")
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${PROJECT_NAME}-${CPACK_PACKAGE_VERSION}-src")
set(CPACK_SOURCE_IGNORE_FILES "/build/;/.git/;/.idea/;/*.user;")

include(CPack)
