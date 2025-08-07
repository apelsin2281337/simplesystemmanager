QT += core gui widgets charts
CONFIG += c++23 warn_on release dynamic
TARGET = SystemManager
TEMPLATE = app

SOURCES += \
    src/addautostartdialog.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/services.cpp \
    src/temp_files.cpp \
    src/autostart.cpp

HEADERS += \
    include/addautostartdialog.hpp \
    include/mainwindow.h \
    include/resource_monitor.hpp \
    include/services.hpp \
    include/temp_files.hpp \
    include/autostart.hpp

FORMS += \
    mainwindow.ui

TRANSLATIONS += \
    translations/qtguiinterface_en_US.ts \
    translations/qtguiinterface_ru_RU.ts

RESOURCES += resources/translations.qrc

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
#QMAKE_CXXFLAGS += -Wno-unused-parameter

RESOURCES += \
    resources/translations.qrc
