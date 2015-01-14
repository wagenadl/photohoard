// MainWindow.h

#ifndef MAINWINDOW_H

#define MAINWINDOW_H

#include <QMainWindow>
#include "FileBar.h"

class MainWindow: public QMainWindow {
  Q_OBJECT;
public:
  MainWindow(class PhotoDB const &,
             class Scanner *,
             class AutoCache *,
             class Exporter *);
  virtual ~MainWindow();
public slots:
  void fileAction(FileBar::Action);
  void scrollToCurrent();
private:
  class Exporter *exporter;
private:
  class LightTable *lightTable;
  class FileBar *fileBar;
  class LayoutBar *layoutBar;
  class ColorLabelBar *colorLabelBar;
  class FilterBar *filterBar;
  class ExportDialog *exportDialog;
};

#endif
