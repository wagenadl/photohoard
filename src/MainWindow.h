// MainWindow.h

#ifndef MAINWINDOW_H

#define MAINWINDOW_H

#include <QMainWindow>
#include "FileBar.h"

class MainWindow: public QMainWindow {
  Q_OBJECT;
public:
  MainWindow(class PhotoDB *, class Scanner *, class AutoCache *);
  virtual ~MainWindow();
public slots:
  void fileAction(FileBar::Action);
private:
  class LightTable *lightTable;
  class FileBar *fileBar;
  class LayoutBar *layoutBar;
  class ColorLabelBar *colorLabelBar;
  class FilterBar *filterBar;
};

#endif
