######################################################################
# Automatically generated by qmake (2.01a) Tue Jan 20 20:25:48 2015
######################################################################

TEMPLATE = app
TARGET = 
CONFIG += debug
DEPENDPATH += .
INCLUDEPATH += .
QMAKE_CXXFLAGS += -std=c++11	
LIBS += -L/tmp/lcms2-2.5/src/.libs -llcms2 -lX11

# Input
HEADERS += Image.h ImageSpaceConverters.h ../CMSProfile.h ../CMSTransform.h
SOURCES += Image.cpp ImageSpaceConverters.cpp test.cpp
SOURCES += ../CMSProfile.cpp ../CMSTransform.cpp