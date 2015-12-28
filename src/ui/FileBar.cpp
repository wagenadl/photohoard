// FileBar.cpp

#include "FileBar.h"

#include "Exporter.h"
#include "Scanner.h"
#include "PDebug.h"
#include "ExportDialog.h"
#include "AddRootDialog.h"
#include <QDir>

FileBar::FileBar(PhotoDB *db, Exporter *exporter,
                 Scanner *scanner, QWidget *parent): QToolBar(parent) {
  exportdialog = 0;
  setWindowTitle("File");

  actions << Action{Qt::CTRL + Qt::SHIFT + Qt::Key_R, "Add new folder tree",
      [=]() {
      AddRootDialog dlg(db);
      while (dlg.exec()) {
        if (dlg.validate(true)) {
          if (!dlg.path().isEmpty() && !dlg.defaultCollection().isEmpty()
              && !dlg.exclusions().contains(dlg.path())) {
            QDir root(dlg.path());
            if (root.exists())
              scanner->addTree(root.absolutePath(), dlg.defaultCollection(),
                               dlg.exclusions());
          }
          break;
      }
      }
    }};
  new PAction(actions.last(), QIcon(":icons/folderAdd.svg"), this);

  actions << Action{Qt::CTRL + Qt::Key_R, "Rescan folders",
      [=]() { scanner->rescanAll(); }};
  new PAction(actions.last(), QIcon(":icons/rescan.svg"), this);

  actions << Action{Qt::CTRL + Qt::Key_I, "Import from camera or card",
      []() { qDebug() << "Not yet implemented"; }};
  new PAction(actions.last(), QIcon(":icons/cameraImport.svg"), this);

  actions << Action{Qt::CTRL + Qt::SHIFT + Qt::Key_E, "Export dialog",
      [exporter,this]() {
      if (!exportdialog)
        exportdialog = new ExportDialog();
      if (exportdialog->exec() == ExportDialog::Accepted) {
        exporter->setup(exportdialog
                        ? exportdialog->settings()
                        : ExportSettings());
        exporter->addSelection();
      }
    }};
  new PAction(actions.last(), QIcon(":icons/export.svg"), this);

  actions << Action{Qt::CTRL + Qt::Key_E, "Export more images",
      [exporter,this]() {
      exporter->setup(exportdialog
                      ? exportdialog->settings()
                      : ExportSettings());
      exporter->addSelection();
    }};
  parent->addAction(new PAction(actions.last(), this));

  actions << Action{Qt::CTRL + Qt::SHIFT + Qt::Key_C, "Clipboard dialog",
      []() { qDebug() << "Not yet implemented"; }};
  parent->addAction(new PAction(actions.last(), this));
}

