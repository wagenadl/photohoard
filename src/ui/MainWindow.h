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
  MainWindow(class SessionDB *,
             class Scanner *,
             class AutoCache *,
             class Exporter *);
  virtual ~MainWindow();
  static void setStatusMessage(QString msg, QWidget *src);
public slots:
  void scrollToCurrent();
  void updateImage(quint64 vsn, Image16 img, quint64 chgid, QSize fullsize);
  void showShortcutHelp();
  void setStatusMessage(QString msg);
protected:
  virtual void closeEvent(QCloseEvent *) override;
  virtual void dragEnterEvent(QDragEnterEvent *) override;
  virtual void dragLeaveEvent(QDragLeaveEvent *) override;
  virtual void dragMoveEvent(QDragMoveEvent *) override;
  virtual void dropEvent(QDropEvent *) override;
private slots:
  void reportExportResults(QString dst, int nOK, int nFail);
  void reloadVersion(quint64 vsn);
private:
  class SessionDB *db;
  class Scanner *scanner;
  class Exporter *exporter;
private:
  class LightTable *lightTable;
  class FileBar *fileBar;
  class LayoutBar *layoutBar;
  class ColorLabelBar *colorLabelBar;
  class FilterBar *filterBar;
  class HelpBar *helpBar;
  class DatabaseBar *databaseBar;
  class AllControls *allControls;
  class HistoWidget *histogram;
  class LiveAdjuster *adjuster;
  class MetaViewer *metaViewer;
  class AppliedTagList *tagList;
  class StatusBar *statusBar;
  class ShortcutHelp *shortcutHelp;
private:
  bool dragout;
  bool dragin;
};


#endif
