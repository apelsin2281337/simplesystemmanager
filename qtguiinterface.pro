QT += core gui widgets charts
CONFIG += c++23 warn_on release dynamic
TARGET = SystemManager
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    services.cpp \
    temp_files.cpp

HEADERS += \
    mainwindow.h \
    resource_monitor.hpp \
    services.hpp \
    temp_files.hpp

FORMS += \
    mainwindow.ui

# Проверка ОС для systemd
linux {
    LIBS += -lsystemd
}

# Настройки C++23 (с фолбэком)
isEqual(QT_MAJOR_VERSION, 6) {
    QMAKE_CXXFLAGS += -std=c++2b
} else {
    QMAKE_CXXFLAGS += -std=c++17
}

# Отключение предупреждений
QMAKE_CXXFLAGS += -Wno-unused-parameter
