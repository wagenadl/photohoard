// MainWindow.h

#ifndef MAINWINDOW_H

#define MAINWINDOW_H

#include <QMainWindow>
#include "FileBar.h"
#include "Image16.h"
#include "LayoutBar.h"

class MainWindow: public QMainWindow {
  Q_OBJECT;
public:
  MainWindow(class PhotoDB *,
             class Scanner *,
             class AutoCache *,
             class Exporter *);
  virtual ~MainWindow();
public slots:
  void fileAction(FileBar::Action);
  void scrollToCurrent();
  void updateImage(quint64, QSize, Image16);
  void setLayout(LayoutBar::Action);
private:
  class Exporter *exporter;
private:
  class LightTable *lightTable;
  class FileBar *fileBar;
  class LayoutBar *layoutBar;
  class ColorLabelBar *colorLabelBar;
  class FilterBar *filterBar;
  class ExportDialog *exportDialog;
  class AllControls *allControls;
  class HistoWidget *histogram;
  class LiveAdjuster *adjuster;
  class MetaViewer *metaViewer;
  class AppliedTagList *tagList;
  class StatusBar *statusBar;
};


#endif
