TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -pthread

SOURCES += \
        main.cpp

CONFIG(debug, debug|release) {
    message("debug")
    DEFINES += _DEBUG
}

QMAKE_CXXFLAGS += -O2
