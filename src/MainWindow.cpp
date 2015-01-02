// MainWindow.cpp

#include "MainWindow.h"
#include "LightTable.h"
#include "LayoutBar.h"
#include "ColorLabelBar.h"
#include "Scanner.h"
#include "AutoCache.h"
#include "PhotoDB.h"

MainWindow::MainWindow(PhotoDB *db, Scanner *scanner, AutoCache *autocache) {
  setCentralWidget(lightTable = new LightTable(*db, this));
  addToolBar(layoutBar = new LayoutBar(this));
  addToolBar(colorLabelBar = new ColorLabelBar(this));
  // etc.

  connect(lightTable, SIGNAL(needImage(quint64, QSize)),
          autocache, SLOT(request(quint64, QSize)));
  connect(autocache, SIGNAL(available(quint64, QSize, QImage)),
          lightTable, SLOT(updateImage(quint64, QSize, QImage)));
  connect(scanner, SIGNAL(updated(QSet<quint64>)),
          lightTable, SLOT(rescan()));
}

  
MainWindow::~MainWindow() {
}
