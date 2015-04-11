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
#include "MetaViewer.h"
#include "StatusBar.h"
#include "AppliedTagList.h"

#include "PDebug.h"

MainWindow::MainWindow(PhotoDB *db,
                       Scanner *scanner, AutoCache *autocache,
                       Exporter *exporter): exporter(exporter) {
  exportDialog = 0;

  QDockWidget *dock = new QDockWidget("Histogram", this);
  dock->setWidget(histogram = new HistoWidget(this));
  dock->setTitleBarWidget(new QWidget());
  addDockWidget(Qt::RightDockWidgetArea, dock);
  
  dock = new QDockWidget("Adjustments", this);
  dock->setWidget(allControls = new AllControls(this));
  dock->setTitleBarWidget(new QWidget());
  addDockWidget(Qt::RightDockWidgetArea, dock);

  dock = new QDockWidget("Metadata",this);
  dock->setWidget(metaViewer = new MetaViewer(db, this));
  dock->setTitleBarWidget(new QWidget());
  addDockWidget(Qt::RightDockWidgetArea, dock);

  dock = new QDockWidget("Tags",this);
  dock->setWidget(tagList = new AppliedTagList(db, this));
  dock->setTitleBarWidget(new QWidget());
  addDockWidget(Qt::RightDockWidgetArea, dock);

  dock = new QDockWidget("Status",this);
  dock->setWidget(statusBar = new StatusBar(this));
  dock->setTitleBarWidget(new QWidget());
  addDockWidget(Qt::RightDockWidgetArea, dock);
  
  adjuster = new LiveAdjuster(db, allControls, autocache, this);
  
  setCentralWidget(lightTable = new LightTable(db, adjuster, this));
  constexpr Qt::ToolBarArea area = Qt::TopToolBarArea;
  addToolBar(area, fileBar = new FileBar(this));
  addToolBar(area, layoutBar = new LayoutBar(this));
  addToolBar(area, colorLabelBar = new ColorLabelBar(this));
  addToolBar(area, filterBar = new FilterBar(this));
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
          this, SLOT(setLayout(LayoutBar::Action)));
  connect(fileBar, SIGNAL(triggered(FileBar::Action)),
	  SLOT(fileAction(FileBar::Action)));
  connect(colorLabelBar, SIGNAL(triggered(ColorLabelBar::Action)),
	  lightTable, SLOT(setColorLabel(ColorLabelBar::Action)));
  connect(filterBar, SIGNAL(triggered(FilterBar::Action)),
	  lightTable, SLOT(filterAction(FilterBar::Action)));
  autocache->requestIfEasy(lightTable->current(), QSize(1024, 1024));

  connect(lightTable, SIGNAL(newCurrent(quint64)),
          metaViewer, SLOT(setVersion(quint64)));
  connect(lightTable, SIGNAL(newZoom(double)),
          statusBar, SLOT(setZoom(double)));

  connect(lightTable, SIGNAL(newCurrent(quint64)),
	  tagList, SLOT(setCurrent(quint64)));
  connect(lightTable, SIGNAL(newSelection()),
	  tagList, SLOT(newSelection()));

  connect(lightTable, SIGNAL(newCurrent(quint64)),
          histogram, SLOT(setVersion(quint64)));
  
  tagList->setCurrent(lightTable->current());
  metaViewer->setVersion(lightTable->current());
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
    pDebug() << "Export selected";
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

void MainWindow::setLayout(LayoutBar::Action a) {
//  if (a==LayoutBar::Action::FullGrid) {
//    histogram->hide();
//    allControls->hide();
//  } else {
//    histogram->show();
//    allControls->show();
//  }

  lightTable->setLayout(a);
}
