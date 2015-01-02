// MainWindow.h

#ifndef MAINWINDOW_H

#define MAINWINDOW_H

#include <QMainWindow>

class MainWindow: public QMainWindow {
  Q_OBJECT;
public:
  MainWindow(class PhotoDB *, class Scanner *, class AutoCache *);
  virtual ~MainWindow();
private:
  class LightTable *lightTable;
  class LayoutBar *layoutBar;
  class ColorLabelBar *colorLabelBar;
};

#endif
