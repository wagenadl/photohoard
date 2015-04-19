// FileBar.h

#ifndef FILEBAR_H

#define FILEBAR_H

#include "ActionBar.h"

class FileBar: public ActionBar {
  Q_OBJECT;
public:
  enum class Action {
    AddFolder,
      RescanFolders,
      ImportFromCamera,
      OpenExportDialog,
      ExportSelected,
      N
      };
public:
  FileBar(QWidget *parent);
  virtual ~FileBar();
signals:
  void triggered(FileBar::Action a);
private slots:
  void trigger(QAction *);
private:
  QMap<Action, QAction *> actions;
  QMap<QAction *, Action> revmap;
};

#endif
