# QMake project file for photohoard                -*- mode: shell-script; -*-

TEMPLATE = app
TARGET = photohoard
CONFIG += debug_and_release
QT += sql widgets x11extras dbus
QMAKE_CXXFLAGS += -std=c++11

QMAKE_CXXFLAGS_DEBUG += -fPIE -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3 -ffast-math $$(PHOTOHOARD_CXXFLAGS)

CONFIG(debug, debug|release) { TARGET=$${TARGET}_debug }

# Input

RESOURCES += PhotoHoard.qrc
RESOURCES += Icons.qrc

LIBS += -lexiv2
LIBS += -llcms2
LIBS += -lopencv_imgproc -lopencv_core
LIBS += -lX11
LIBS += -lxcb-randr -lxcb

include(photohoard.pri)
DEPENDPATH +=  . $$sourcedirs
INCLUDEPATH += . $$sourcedirs
INCLUDEPATH += /usr/include/opencv4
for(sd, sourcedirs): include(../src/$${sd}/$${sd}.pri)
