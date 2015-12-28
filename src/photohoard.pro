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

DEPENDPATH += . ./db ./ui ./cms ./adj
INCLUDEPATH += . ./db ./ui ./cms ./adj

# Input


RESOURCES += PhotoHoard.qrc

FORMS += ui/ExportDialog.ui  ui/FilterDialog.ui  ui/AddRootDialog.ui

LIBS += -lexiv2
LIBS += -llcms2
LIBS += -lopencv_imgproc -lopencv_core
LIBS += -lX11

SOURCES += cms/CMSProfile.cpp
SOURCES += cms/CMSTransform.cpp
SOURCES += db/AC_Worker.cpp
SOURCES += db/AutoCache.cpp
SOURCES += db/BasicCache.cpp
SOURCES += db/BasicThread.cpp
SOURCES += db/Database.cpp
SOURCES += db/ExceptionReporter.cpp
SOURCES += db/Exif.cpp
SOURCES += db/ExifReport.cpp
SOURCES += db/Exporter.cpp
SOURCES += db/ExportSettings.cpp
SOURCES += db/IF_Bank.cpp
SOURCES += db/IF_Worker.cpp
SOURCES += db/ImageFinder.cpp
SOURCES += db/NiceProcess.cpp
SOURCES += db/NikonLenses.cpp db/CanonLenses.cpp
SOURCES += db/PhotoDB.cpp
SOURCES += db/Scanner.cpp
SOURCES += db/Selection.cpp
SOURCES += db/SqlFile.cpp
SOURCES += ui/ActionBar.cpp
SOURCES += ui/Application.cpp
SOURCES += ui/ColorLabelBar.cpp
SOURCES += ui/Datestrip.cpp
SOURCES += ui/ExportDialog.cpp
SOURCES += ui/FileBar.cpp
SOURCES += ui/StripScene.cpp
SOURCES += ui/StripView.cpp
SOURCES += ui/FilterBar.cpp
SOURCES += ui/GentleJog.cpp
SOURCES += ui/LayoutBar.cpp
SOURCES += ui/LightTable.cpp
SOURCES += ui/main.cpp
SOURCES += ui/MainWindow.cpp
SOURCES += ui/ProgressWidget.cpp
SOURCES += ui/Slide.cpp
SOURCES += ui/Slidestrip.cpp
SOURCES += ui/SlideView.cpp
SOURCES += ui/Strip.cpp

HEADERS += cms/CMS.h
HEADERS += cms/CMSProfile.h
HEADERS += cms/CMSTransform.h
HEADERS += db/AC_Worker.h
HEADERS += db/AutoCache.h
HEADERS += db/BasicCache.h
HEADERS += db/BasicThread.h
HEADERS += db/Database.h
HEADERS += db/ExceptionReporter.h
HEADERS += db/Exif.h
HEADERS += db/ExifReport.h
HEADERS += db/Exporter.h
HEADERS += db/ExportSettings.h
HEADERS += db/IF_Bank.h
HEADERS += db/IF_Worker.h
HEADERS += db/ImageFinder.h
HEADERS += db/NiceProcess.h
HEADERS += db/NikonLenses.h db/CanonLenses.h
HEADERS += db/NoResult.h
HEADERS += db/PhotoDB.h
HEADERS += db/Scanner.h
HEADERS += db/Selection.h
HEADERS += db/SqlFile.h
HEADERS += ui/ActionBar.h
HEADERS += ui/Application.h
HEADERS += ui/ColorLabelBar.h
HEADERS += ui/Datestrip.h
HEADERS += ui/ExportDialog.h
HEADERS += ui/FileBar.h
HEADERS += ui/StripScene.h
HEADERS += ui/StripView.h
HEADERS += ui/FilterBar.h
HEADERS += ui/GentleJog.h
HEADERS += ui/LayoutBar.h
HEADERS += ui/LightTable.h
HEADERS += ui/MainWindow.h
HEADERS += ui/ProgressWidget.h
HEADERS += ui/Slide.h
HEADERS += ui/Slidestrip.h
HEADERS += ui/SlideView.h
HEADERS += ui/Strip.h

HEADERS += db/PDebug.h
SOURCES += db/PDebug.cpp
SOURCES += adj/Sliders.cpp
HEADERS += adj/Sliders.h
HEADERS += adj/ColorSpaces.h   adj/CS_IPT.h   adj/CS_Lab.h   adj/CS_sRGB.h
SOURCES += adj/ColorSpaces.cpp adj/CS_IPT.cpp adj/CS_Lab.cpp adj/CS_sRGB.cpp
HEADERS += adj/Image16.h adj/Image16Base.h adj/Image16Data.h
HEADERS += adj/Adjuster.h Clarity.h clahe.c
SOURCES += adj/Image16.cpp adj/Adjuster.cpp Clarity.cpp
HEADERS += ui/AllControls.h   ui/ControlGroup.h
SOURCES += ui/AllControls.cpp ui/ControlGroup.cpp
HEADERS += adj/AdjusterTile.h   adj/AdjusterStage.h
SOURCES += adj/AdjusterTile.cpp adj/AdjusterStage.cpp
HEADERS += adj/AdjusterXYZ.h   adj/AdjusterIPT.h   adj/AdjusterGeometry.h
SOURCES += adj/AdjusterXYZ.cpp adj/AdjusterIPT.cpp adj/AdjusterGeometry.cpp
HEADERS += adj/AdjusterUMask.h   adj/AdjusterEqualize.h
SOURCES += adj/AdjusterUMask.cpp adj/AdjusterEqualize.cpp
HEADERS += adj/Histogram.h   adj/InterruptableAdjuster.h
SOURCES += adj/Histogram.cpp adj/InterruptableAdjuster.cpp
HEADERS += ui/HistoWidget.h
SOURCES += ui/HistoWidget.cpp
HEADERS += db/OriginalFinder.h
SOURCES += db/OriginalFinder.cpp
HEADERS += ui/LiveAdjuster.h
SOURCES += ui/LiveAdjuster.cpp
HEADERS += db/InterruptableReader.h
HEADERS += db/InterruptableRawReader.h db/InterruptableFileReader.h
SOURCES += db/InterruptableReader.cpp
SOURCES += db/InterruptableRawReader.cpp db/InterruptableFileReader.cpp
HEADERS += db/PSize.h
SOURCES += db/PSize.cpp
HEADERS += ui/MetaViewer.h
SOURCES += ui/MetaViewer.cpp
HEADERS += db/AC_ImageHolder.h
SOURCES += db/AC_ImageHolder.cpp
HEADERS += adj/PPM16.h
SOURCES += adj/PPM16.cpp
HEADERS += ui/StatusBar.h 
HEADERS += ui/TagDialog.h ui/TagListWidget.h
SOURCES += ui/StatusBar.cpp
SOURCES += ui/TagDialog.cpp ui/TagListWidget.cpp
HEADERS += db/Tags.h db/MetaInfo.h     
SOURCES += db/Tags.cpp  db/MetaInfo.cpp 
HEADERS += ui/AppliedTagList.h   ui/AppliedTagWidget.h   ui/AppliedTagEditor.h
SOURCES += ui/AppliedTagList.cpp ui/AppliedTagWidget.cpp ui/AppliedTagEditor.cpp
HEADERS += ui/FilterDialog.h   db/Filter.h   ui/AddRootDialog.h
SOURCES += ui/FilterDialog.cpp db/Filter.cpp ui/AddRootDialog.cpp
HEADERS += db/PurgeCache.h
SOURCES += db/PurgeCache.cpp
HEADERS += ui/SliderGroups.h   SliderClipboard.h
SOURCES += ui/SliderGroups.cpp SliderClipboard.cpp
HEADERS += ui/Action.h
SOURCES += ui/Action.cpp
HEADERS += ui/ShortcutHelp.h
SOURCES += ui/ShortcutHelp.cpp
