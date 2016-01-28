# QMake project file for photohoard                -*- mode: shell-script; -*-

TEMPLATE = app
TARGET = ../photohoard
CONFIG += debug_and_release
QT += sql
QMAKE_CXXFLAGS += -std=c++11

QMAKE_CXXFLAGS_DEBUG += -fPIE -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3 -ffast-math -march=native
#QMAKE_CXXFLAGS_DEBUG -= -O2
#QMAKE_CXXFLAGS_DEBUG += -O0
#QMAKE_CXXFLAGS += -fdiagnostics-color=never

CONFIG(debug, debug|release) { TARGET=$${TARGET}_debug }

OBJECTS_DIR=../build/release
CONFIG(debug, debug|release) { OBJECTS_DIR=../build/debug }
MOC_DIR = $${OBJECTS_DIR}
RCC_DIR = $${OBJECTS_DIR}
UI_DIR = $${OBJECTS_DIR}


# Input

RESOURCES += PhotoHoard.qrc

LIBS += -lexiv2
LIBS += -llcms2
LIBS += -lopencv_imgproc -lopencv_core
LIBS += -lX11

include(photohoard.pri)
DEPENDPATH +=  . $$sourcedirs
INCLUDEPATH += . $$sourcedirs
for(sd, sourcedirs): include($${sd}/$${sd}.pri)
