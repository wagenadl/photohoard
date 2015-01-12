// FileBar.cpp

#include "FileBar.h"

#include <QMetaType>
#include <QDebug>

FileBar::FileBar(QWidget *parent): QToolBar(parent) {
  qRegisterMetaType<FileBar::Action>("FileBar::Action");

  setWindowTitle("File");
  
  for (int i=0; i<int(Action::N); i++) {
    Action ii = Action(i);
    QAction *a = new QAction(parent);
    actions[ii] = a;
    revmap[a] = ii;
  }

  actions[Action::AddFolder]->setIcon(QIcon(":icons/folderAdd.svg"));
  actions[Action::ImportCamera]->setIcon(QIcon(":icons/cameraImport.svg"));
  actions[Action::OpenExportDialog]->setIcon(QIcon(":icons/export.svg"));

  actions[Action::AddFolder]->setText("(Re-)scan folder (Control-R)");
  actions[Action::ImportCamera]
    ->setText("Import from camera or card (Control-I)");
  actions[Action::OpenExportDialog]->setText("Export JPEG(s) (Control-E)");

  actions[Action::AddFolder]->setShortcut(QString("Control+R"));
  actions[Action::ImportCamera]->setShortcut(QString("Control+I"));
  actions[Action::OpenExportDialog]->setShortcut(QString("Control+E"));

  addAction(actions[Action::ImportCamera]);
  addAction(actions[Action::AddFolder]);
  addAction(actions[Action::OpenExportDialog]);

  connect(this, SIGNAL(actionTriggered(QAction*)),
          SLOT(trigger(QAction*)));
}

FileBar::~FileBar() {
}



void FileBar::trigger(QAction *a) {
  qDebug() << "FileBar::trigger" << a << revmap.contains(a);
  if (revmap.contains(a))
    emit triggered(revmap[a]);
}
