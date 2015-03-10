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
#include "AllControls.h"
#include "HistoWidget.h"
#include <QDockWidget>
#include "LiveAdjuster.h"

#include <QDebug>

MainWindow::MainWindow(PhotoDB const &db,
                       Scanner *scanner, AutoCache *autocache,
                       Exporter *exporter): exporter(exporter) {
  exportDialog = 0;

  QDockWidget *dock = new QDockWidget("Histogram", this);
  dock->setWidget(histogram = new HistoWidget(this));
  addDockWidget(Qt::RightDockWidgetArea, dock);
  
  dock = new QDockWidget("Adjustments", this);
  dock->setWidget(allControls = new AllControls(this));
  addDockWidget(Qt::RightDockWidgetArea, dock);

  adjuster = new LiveAdjuster(db, allControls, autocache, this);
  
  setCentralWidget(lightTable = new LightTable(db, adjuster, this));
  addToolBar(fileBar = new FileBar(this));
  addToolBar(layoutBar = new LayoutBar(this));
  addToolBar(colorLabelBar = new ColorLabelBar(this));
  addToolBar(filterBar = new FilterBar(this));
  // etc.
  
  connect(adjuster, SIGNAL(imageChanged(Image16, quint64)),
          histogram, SLOT(setImage(Image16))); // is this ok?

  connect(lightTable, SIGNAL(needImage(quint64, QSize)),
          autocache, SLOT(request(quint64, QSize)));
  connect(autocache, SIGNAL(available(quint64, QSize, Image16)),
          SLOT(updateImage(quint64, QSize, Image16)));
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
  autocache->request(lightTable->current(), QSize(1024, 1024)); // hack
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

void MainWindow::scrollToCurrent() {
  lightTable->scrollToCurrent();
}

void MainWindow::updateImage(quint64 i, QSize, Image16 img) {
  lightTable->updateImage(i, img);
  if (i==lightTable->current())
    histogram->setImage(img);
}

