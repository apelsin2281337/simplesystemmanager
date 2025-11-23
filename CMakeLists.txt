cmake_minimum_required(VERSION 3.16)
project(SystemManager)

# Поиск последней версии Qt
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

# Выходные директории
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIREFCTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_AUTOGEN_BUILD_DIR ${CMAKE_BINARY_DIR}/autogen)
set(CMAKE_RCC_BUILD_DIR ${CMAKE_BINARY_DIR}/rcc)

# Qt автоматизация
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR})
set(Qt6_LUPDATE_EXECUTABLE "/usr/bin/lupdate")


# Qt компоненты
find_package(Qt6 REQUIRED COMPONENTS
    Core Gui Widgets Charts LinguistTools
)
find_package(Qt6 COMPONENTS WaylandClient)

set(CMAKE_CXX_STANDARD 23)

if(TARGET Qt6::WaylandClient)
    add_compile_definitions(QT_WAYLAND)
    message(STATUS "Wayland support enabled")
else()
    message(WARNING "Wayland support not found - building with X11 only")
endif()

# Исходники
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
    src/taskmanager_controller.cpp
    src/autostart_controller.cpp
    src/services_controller.cpp
    src/tempfiles_controller.cpp
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
    include/mainincludes.hpp
    include/taskmanager_controller.hpp
    include/tempfiles_controller.hpp
    include/autostart_controller.hpp
    include/services_controller.hpp
)

set(FORMS mainwindow.ui)

set(RESOURCES
    resources/icon.qrc
    resources/themes.qrc
    resources/transtations.qrc
)

# Переводы
set(TS_FILES
    translations/qtguiinterface_en_US.ts
    translations/qtguiinterface_ru_RU.ts
)

set(TRANSLATION_SOURCES
    ${APP_SOURCES}
    ${HEADERS}
    ${FORMS}
)

# Проверка наличия lupdate
if(NOT Qt6_LUPDATE_EXECUTABLE)
    message(FATAL_ERROR "Qt6_LUPDATE_EXECUTABLE not found. Make sure Qt LinguistTools is installed.")
endif()

if(DOWNLOAD_MISSING_DEPS)
    if(NOT TARGET nlohmann_json::nlohmann_json)
        message(STATUS "Downloading nlohmann/json...")
        FetchContent_Declare(
            nlohmann_json
            GIT_REPOSITORY https://github.com/nlohmann/json.git
            GIT_TAG v3.11.2
        )
        FetchContent_MakeAvailable(nlohmann_json)
    endif()
endif()

# Цель для ручного обновления .ts файлов
add_custom_target(update_translations
    COMMAND ${Qt6_LUPDATE_EXECUTABLE}
            ${TRANSLATION_SOURCES}
            -ts ${TS_FILES}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Updating translation source files (.ts)"
)

# Компиляция .ts → .qm
qt6_add_translation(QM_FILES ${TS_FILES})

# Цель для перевода
add_custom_target(translations ALL DEPENDS ${QM_FILES})

# Основной исполняемый файл
add_executable(SystemManager
    src/main.cpp
    ${APP_SOURCES}
    ${HEADERS}
    ${FORMS}
    ${RESOURCES}
    include/mainincludes.hpp
)

target_link_libraries(SystemManager
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Charts
)

if(TARGET Qt6::WaylandClient)
    target_link_libraries(SystemManager Qt6::WaylandClient)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(SystemManager systemd)
endif()

add_dependencies(SystemManager translations)

# Установка
install(TARGETS SystemManager DESTINATION bin)
install(FILES ${QM_FILES} DESTINATION share/${PROJECT_NAME}/translations)

# .desktop файл
configure_file(
    packaging/SystemManager.desktop
    ${CMAKE_CURRENT_BINARY_DIR}/SystemManager.desktop
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/SystemManager.desktop
    DESTINATION share/applications
)
