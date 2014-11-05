# QMake project file for photohoard                -*- mode: shell-script; -*-

TEMPLATE = app
TARGET = photohoard
CONFIG += debug_and_release
QT += sql
QMAKE_CXXFLAGS += -std=c++11

CONFIG(debug, debug|release) { TARGET=$${TARGET}_debug }

DEPENDPATH += .
INCLUDEPATH += .

# Input
HEADERS += AutoCache.h BasicCache.h ThreadedCache.h
SOURCES += BasicCache.cpp
SOURCES += main.cpp
RESOURCES += PhotoHoard.qrc
