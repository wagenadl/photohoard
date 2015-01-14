// MainWindow.cpp

#include "MainWindow.h"
#include "LightTable.h"
#include "LayoutBar.h"
#include "FileBar.h"
#include "FilterBar.h"
#include "ColorLabelBar.h"
#include "Scanner.h"
#include "AutoCache.h"
#include "PhotoDB.h"
#include "ExportDialog.h"
#include "Exporter.h"

#include <QDebug>

MainWindow::MainWindow(PhotoDB const &db,
                       Scanner *scanner, AutoCache *autocache,
                       Exporter *exporter): exporter(exporter) {
  exportDialog = 0;
  
  setCentralWidget(lightTable = new LightTable(db, this));
  addToolBar(fileBar = new FileBar(this));
  addToolBar(layoutBar = new LayoutBar(this));
  addToolBar(colorLabelBar = new ColorLabelBar(this));
  addToolBar(filterBar = new FilterBar(this));
  // etc.

  connect(lightTable, SIGNAL(needImage(quint64, QSize)),
          autocache, SLOT(request(quint64, QSize)));
  connect(autocache, SIGNAL(available(quint64, QSize, QImage)),
          lightTable, SLOT(updateImage(quint64, QSize, QImage)));
  connect(scanner, SIGNAL(updated(QSet<quint64>)),
          lightTable, SLOT(rescan()));

  connect(layoutBar, SIGNAL(triggered(LayoutBar::Action)),
          lightTable, SLOT(setLayout(LayoutBar::Action)));
  connect(fileBar, SIGNAL(triggered(FileBar::Action)),
	  SLOT(fileAction(FileBar::Action)));
  connect(colorLabelBar, SIGNAL(triggered(ColorLabelBar::Action)),
	  lightTable, SLOT(setColorLabel(ColorLabelBar::Action)));
  connect(filterBar, SIGNAL(triggered(FilterBar::Action)),
	  lightTable, SLOT(filterAction(FilterBar::Action)));
}

  
MainWindow::~MainWindow() {
}

void MainWindow::fileAction(FileBar::Action a) {
  switch (a) {
  case FileBar::Action::OpenExportDialog:
    if (!exportDialog)
      exportDialog = new ExportDialog();
    if (exportDialog->exec() == ExportDialog::Accepted)
      fileAction(FileBar::Action::ExportSelected);
    break;
  case FileBar::Action::ExportSelected:
    qDebug() << "Export selected";
    exporter->setup(exportDialog ? exportDialog->settings() : ExportSettings());
    exporter->addSelection();
    break;
  default:
    break;
  }
}
