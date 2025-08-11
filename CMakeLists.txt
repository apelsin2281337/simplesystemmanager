cmake_minimum_required(VERSION 3.16)
project(SystemManager)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Charts LinguistTools)

set(CMAKE_CXX_STANDARD 23)


set(SOURCES
    src/addautostartdialog.cpp
    src/logger.cpp
    src/main.cpp
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
)

add_executable(${PROJECT_NAME}
    ${SOURCES}
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
