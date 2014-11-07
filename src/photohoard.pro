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
HEADERS += BasicCache.h   Exif.h   NikonLenses.h   
SOURCES += BasicCache.cpp Exif.cpp NikonLenses.cpp 
HEADERS += Database.h   PhotoDB.h   SqlFile.h   Scanner.h
SOURCES += Database.cpp PhotoDB.cpp SqlFile.cpp Scanner.cpp
SOURCES += main.cpp
RESOURCES += PhotoHoard.qrc
LIBS += -lexiv2
