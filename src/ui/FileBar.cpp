// FileBar.cpp

#include "FileBar.h"

#include <QMetaType>
#include "PDebug.h"

FileBar::FileBar(QWidget *parent): ActionBar(parent) {
  qRegisterMetaType<FileBar::Action>("FileBar::Action");

  setWindowTitle("File");
  
  for (int i=0; i<int(Action::N); i++) {
    Action ii = Action(i);
    QAction *a = new QAction(parent);
    actions[ii] = a;
    revmap[a] = ii;
  }

  actions[Action::AddFolder]->setIcon(QIcon(":icons/folderAdd.svg"));
  actions[Action::RescanFolders]->setIcon(QIcon(":icons/rescan.svg"));
  actions[Action::ImportFromCamera]->setIcon(QIcon(":icons/cameraImport.svg"));
  actions[Action::OpenExportDialog]->setIcon(QIcon(":icons/export.svg"));

  actions[Action::AddFolder]->setText("Add new folder tree");
  actions[Action::RescanFolders]->setText("Rescan folders (Control-R)");
  actions[Action::ImportFromCamera]
    ->setText("Import from camera or card (Control-I)");
  actions[Action::OpenExportDialog]->setText("Export image(s) (Control-Shift-E)");

  actions[Action::RescanFolders]->setShortcut(QString("Ctrl+R"));
  actions[Action::ImportFromCamera]->setShortcut(QString("Ctrl+I"));
  actions[Action::OpenExportDialog]
    ->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_E));
  actions[Action::ExportSelected]
    ->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_E));

  addAction(actions[Action::ImportFromCamera]);
  addAction(actions[Action::AddFolder]);
  addAction(actions[Action::RescanFolders]);
  addAction(actions[Action::OpenExportDialog]);

  addHiddenAction(actions[Action::ExportSelected]);
}

FileBar::~FileBar() {
}



void FileBar::trigger(QAction *a) {
  pDebug() << "FileBar::trigger" << a << revmap.contains(a);
  if (revmap.contains(a))
    emit triggered(revmap[a]);
}


