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
HEADERS += BasicCache.h   Exif.h   NikonLenses.h   BasicThread.h
SOURCES += BasicCache.cpp Exif.cpp NikonLenses.cpp BasicThread.cpp
HEADERS += Database.h   PhotoDB.h   SqlFile.h   Scanner.h  
SOURCES += Database.cpp PhotoDB.cpp SqlFile.cpp Scanner.cpp
HEADERS += IF_Bank.h   ImageFinder.h   IF_Worker.h
SOURCES += IF_Bank.cpp ImageFinder.cpp IF_Worker.cpp
HEADERS += ProgressWidget.h   AutoCache.h   AC_Worker.h   ExceptionReporter.h
SOURCES += ProgressWidget.cpp AutoCache.cpp AC_Worker.cpp ExceptionReporter.cpp
HEADERS += Application.h   ExifReport.h   NiceProcess.h
SOURCES += Application.cpp ExifReport.cpp NiceProcess.cpp main.cpp
HEADERS += Slide.h   FilmView.h   SlideView.h   LightTable.h
SOURCES += Slide.cpp FilmView.cpp SlideView.cpp LightTable.cpp
HEADERS += Strip.h   Slidestrip.h   Datestrip.h   FilmScene.h
SOURCES += Strip.cpp Slidestrip.cpp Datestrip.cpp FilmScene.cpp
HEADERS += LayoutBar.h   MainWindow.h   ColorLabelBar.h   Selection.h
SOURCES += LayoutBar.cpp MainWindow.cpp ColorLabelBar.cpp Selection.cpp
HEADERS += FileBar.h   FilterBar.h   ExportDialog.h   ActionBar.h
SOURCES += FileBar.cpp FilterBar.cpp ExportDialog.cpp ActionBar.cpp
HEADERS += Exporter.h   ExportSettings.h  
SOURCES += Exporter.cpp ExportSettings.cpp
HEADERS += CMSToneCurve.h   CMSProfile.h   CMSTransform.h
SOURCES += CMSToneCurve.cpp CMSProfile.cpp CMSTransform.cpp
HEADERS += GentleJog.h
SOURCES += GentleJog.cpp

RESOURCES += PhotoHoard.qrc

FORMS += ExportDialog.ui 


LIBS += -lexiv2
LIBS += -llcms2
LIBS += -lX11
