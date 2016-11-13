// FileBar.cpp

#include "FileBar.h"

#include "Exporter.h"
#include "Scanner.h"
#include "PDebug.h"
#include "ExportDialog.h"
#include "AddRootDialog.h"
#include <QDir>
#include "MainWindow.h"
#include "SliderClipboard.h"

FileBar::FileBar(SessionDB *db, AutoCache *ac, Exporter *exporter,
                 Scanner *scanner, MainWindow *parent): ActionBar(parent) {
  setObjectName("File");
  sliderclip = new SliderClipboard(db, ac);

  if (!db->isReadOnly()) {
    acts << Action{Qt::CTRL + Qt::SHIFT + Qt::Key_R, "Add new folder tree",
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
    new PAction(acts.last(), QIcon(":icons/folderAdd.svg"), this);
    
    acts << Action{Qt::CTRL + Qt::Key_R, "Rescan folders",
	[=]() { scanner->rescanAll(); }};
    new PAction(acts.last(), QIcon(":icons/rescan.svg"), this);
    
    acts << Action{Qt::CTRL + Qt::Key_I, "Import from camera or card",
	[]() { COMPLAIN("Import: Not yet implemented"); }};
    new PAction(acts.last(), QIcon(":icons/cameraImport.svg"), this);
  }
  
  acts << Action{Qt::CTRL + Qt::SHIFT + Qt::Key_E, "Export dialog",
      [exporter,this]() {
      ExportDialog::standalone(exporter, true);
    }};
  new PAction(acts.last(), QIcon(":icons/export.svg"), this);

  acts << Action{Qt::CTRL + Qt::Key_E, "Export more images",
      [exporter,this]() {
      if (exporter->settings().isValid()) 
        exporter->addSelection();
      else 
        ExportDialog::standalone(exporter, true);
    }};
  parent->addAction(new PAction(acts.last(), this));
  
  acts << Action{Qt::CTRL + Qt::SHIFT + Qt::Key_2, "Email exported images",
      [exporter,this]() {
      exporter->sendEmail();
    }};
  parent->addAction(new PAction(acts.last(), this));
  
  if (!db->isReadOnly()) {
    acts << Action{Qt::CTRL + Qt::SHIFT + Qt::Key_C, "Clipboard dialog",
	[this]() { this->sliderclip->show(); }};
    parent->addAction(new PAction(acts.last(), this));
  }
  
  acts << Action{Qt::CTRL + Qt::Key_H, "Shortcut help",
      [this]() { auto w = dynamic_cast<MainWindow*>(this->parent());
      Q_ASSERT(w); w->showShortcutHelp(); }};
  parent->addAction(new PAction(acts.last(), this));
}

