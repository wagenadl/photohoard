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

MainWindow::MainWindow(PhotoDB *db, Scanner *scanner, AutoCache *autocache) {
  setCentralWidget(lightTable = new LightTable(*db, this));
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

void MainWindow::fileAction(FileBar::Action) {
  // NYI
}
