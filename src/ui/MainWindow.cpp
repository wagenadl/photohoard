// MainWindow.cpp

#include "MainWindow.h"
#include "LightTable.h"
#include "LayoutBar.h"
#include "FileBar.h"
#include "FilterBar.h"
#include "HelpBar.h"
#include "DatabaseBar.h"
#include "ColorLabelBar.h"
#include "Scanner.h"
#include "AutoCache.h"
#include "SessionDB.h"
#include "AllControls.h"
#include "HistoWidget.h"
#include <QDockWidget>
#include "LiveAdjuster.h"
#include "MetaViewer.h"
#include "StatusBar.h"
#include "AppliedTagList.h"
#include "AddRootDialog.h"
#include <QDir>
#include "PDebug.h"
#include "ShortcutHelp.h"
#include "Exporter.h"
#include <QMessageBox>
#include "Filter.h"
#include "SliderClipboard.h"
#include <QApplication>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QMimeData>
#include "ImportGUI.h"
#include "Settings.h"

MainWindow::MainWindow(SessionDB *db,
                       Scanner *scanner, AutoCache *autocache,
                       Exporter *exporter):
  db(db), scanner(scanner), exporter(exporter) {

  setWindowTitle("Photohoard");
  
  QDockWidget *dock = new QDockWidget("Histogram", this);
  dock->setObjectName("Histogram");
  dock->setWidget(histogram = new HistoWidget(this));
  dock->setTitleBarWidget(new QWidget());
  addDockWidget(Qt::RightDockWidgetArea, dock);
  
  dock = new QDockWidget("Adjustments", this);
  dock->setObjectName("Adjustments");  
  dock->setWidget(allControls = new AllControls(db, this));
  dock->setTitleBarWidget(new QWidget());
  addDockWidget(Qt::RightDockWidgetArea, dock);
  allControls->setVersion(db->current());
  
  dock = new QDockWidget("Metadata",this);
  dock->setObjectName("MetaData");
  dock->setWidget(metaViewer = new MetaViewer(db, this));
  dock->setTitleBarWidget(new QWidget());
  addDockWidget(Qt::RightDockWidgetArea, dock);

  dock = new QDockWidget("Tags",this);
  dock->setObjectName("Tags");
  dock->setWidget(tagList = new AppliedTagList(db, this));
  dock->setTitleBarWidget(new QWidget());
  addDockWidget(Qt::RightDockWidgetArea, dock);

  dock = new QDockWidget("Status",this);
  dock->setObjectName("Status");
  dock->setWidget(statusBar = new StatusBar(db, this));
  dock->setTitleBarWidget(new QWidget());
  addDockWidget(Qt::RightDockWidgetArea, dock);

  adjuster = new LiveAdjuster(db, autocache, this);

  shortcutHelp = new ShortcutHelp();
  //  qDebug() << "help sizehint" << shortcutHelp->sizeHint();

  setCentralWidget(lightTable = new LightTable(db, autocache, adjuster,
                                               exporter, this));

  constexpr Qt::ToolBarArea area = Qt::TopToolBarArea;
  addToolBar(area, fileBar = new FileBar(db, autocache,
                                         exporter, scanner, this));
  addToolBar(area, layoutBar = new LayoutBar(lightTable, this));
  if (!db->isReadOnly())
    addToolBar(area, colorLabelBar = new ColorLabelBar(db, lightTable, this));
  addToolBar(area, filterBar = new FilterBar(lightTable, this));
  // etc.
  addToolBarBreak(area);
  addToolBar(area, helpBar = new HelpBar(this));
  addToolBar(area, databaseBar = new DatabaseBar(db, this));

  shortcutHelp->addSection("General", fileBar->actions());
  shortcutHelp->addSection("General", filterBar->actions());
  //  shortcutHelp->addSection("General", databaseBar->actions());
  shortcutHelp->addSection("Layout", layoutBar->actions());
  if (!db->isReadOnly())
    shortcutHelp->addSection("Labels and marks", colorLabelBar->actions());
  shortcutHelp->addSection("Image strip and photo editor",
                           lightTable->actions());
  if (!db->isReadOnly())
    shortcutHelp->addSection("Slider panel", allControls->actions());
  
  connect(adjuster, SIGNAL(imageAvailable(Image16, quint64, QSize)),
          histogram, SLOT(setImage(Image16))); // is this ok?
  connect(adjuster, SIGNAL(imageAvailable(Image16, quint64, QSize)),
          metaViewer, SLOT(setImage(Image16, quint64)));

  connect(lightTable, SIGNAL(needImage(quint64, QSize)),
          autocache, SLOT(request(quint64, QSize)));
  connect(lightTable, SIGNAL(wantImage(quint64, QSize)),
          autocache, SLOT(requestIfEasy(quint64, QSize)));
  connect(lightTable, SIGNAL(recacheReoriented(QSet<quint64>)),
	  autocache, SLOT(recache(QSet<quint64>)));
  connect(autocache, SIGNAL(available(quint64, Image16, quint64, QSize)),
          SLOT(updateImage(quint64, Image16, quint64, QSize)));
  if (scanner) {
    connect(scanner, SIGNAL(updatedBatch(QSet<quint64>)),
	    lightTable, SLOT(rescan()));
    connect(scanner, SIGNAL(message(QString)),
            statusBar, SLOT(setMessage(QString)));
  }

  autocache->requestIfEasy(db->current(), QSize(1024, 1024));

  connect(lightTable, SIGNAL(newCurrent(quint64)),
          metaViewer, SLOT(setVersion(quint64)));
  connect(lightTable, SIGNAL(newZoom(double)),
          statusBar, SLOT(setZoom(double)));
  connect(lightTable, SIGNAL(newCollection(QString)),
          statusBar, SLOT(setCollection(QString)));

  connect(lightTable, SIGNAL(newCurrent(quint64)),
	  tagList, SLOT(setCurrent(quint64)));
  connect(lightTable, SIGNAL(newSelection()),
	  tagList, SLOT(newSelection()));

  connect(lightTable, SIGNAL(newCurrent(quint64)),
          histogram, SLOT(setVersion(quint64)));

  connect(lightTable, SIGNAL(newCurrent(quint64)),
	  allControls, SLOT(setVersion(quint64)));
  connect(allControls, SIGNAL(valuesChanged(quint64, int, Adjustments)),
	  adjuster, SLOT(reloadSliders(quint64, int, Adjustments)));
  connect(allControls, SIGNAL(maskChanged(quint64, int)),
	  adjuster, SLOT(reloadLayers(quint64, int)));
  connect(allControls, SIGNAL(layerSelected(quint64, int)),
	  lightTable, SLOT(visualizeLayer(quint64, int)));

  connect(exporter, SIGNAL(completed(QString, int, int)),
          SLOT(reportExportResults(QString, int, int)));
  
  tagList->setCurrent(db->current());
  metaViewer->setVersion(db->current());
  { Filter flt(db); flt.loadFromDb();
    if (flt.hasCollection())
      statusBar->setCollection(flt.collection());
  }

  connect(fileBar->sliderClipboard(), SIGNAL(modified(quint64)),
          SLOT(reloadVersion(quint64)));

  connect(metaViewer, SIGNAL(filterModified()),
	  lightTable, SLOT(updateFilterAndDialog()));

  dragout = false;
  dragin = false;
  setAcceptDrops(true);

  Settings s;
  if (s.contains("mwgeom"))
    restoreGeometry(s.get("mwgeom").toByteArray());
  if (s.contains("mwstate"))
    restoreState(s.get("mwstate").toByteArray());
  //  qDebug() << "Mainwindow" << geometry();
  lightTable->resize(size());
  lightTable->restoreSizes();
}

MainWindow::~MainWindow() {
}

void MainWindow::scrollToCurrent() {
  lightTable->scrollToCurrent();
}

void MainWindow::updateImage(quint64 i, Image16 img, quint64 chgid, QSize fs) {
  lightTable->updateImage(i, img, chgid, fs);
  if (i==db->current())
    histogram->setImage(img); // this is *bad*
}

void MainWindow::showShortcutHelp() {
  shortcutHelp->show();
  shortcutHelp->resize(shortcutHelp->sizeHint());
  shortcutHelp->resize(shortcutHelp->sizeHint() + QSize(2, 2));
  shortcutHelp->resize(shortcutHelp->sizeHint() + QSize(2, 2));
}

void MainWindow::reportExportResults(QString dst, int nOK, int nFail) {
  if (nFail) {
    if (nOK==0) 
      QMessageBox::warning(0, "photohoard", 
                           QString::fromUtf8("Export to folder “")
                           + dst + QString::fromUtf8("” failed."));
    else 
      QMessageBox::warning(0, "photohoard", 
                           QString::fromUtf8("Part of export to folder “")
                           + dst + QString::fromUtf8("” failed:")
                           + QString(" %1 failures out of %2")
                           .arg(nFail).arg(nOK+nFail));
  }
}

void MainWindow::setStatusMessage(QString msg, QWidget *src) {
  ASSERT(src);
  MainWindow *mw = dynamic_cast<MainWindow*>(src->window());
  ASSERT(mw);
  mw->setStatusMessage(msg);
}

void MainWindow::setStatusMessage(QString msg) {
  statusBar->setMessage(msg);
}

void MainWindow::reloadVersion(quint64 vsn) {
  if (vsn==db->current()) {
    histogram->setVersion(vsn);
    metaViewer->setVersion(vsn);
  }
  lightTable->reloadVersion(vsn);
}

void MainWindow::closeEvent(QCloseEvent *) {
  Settings s;
  s.set("mwgeom", saveGeometry());
  s.set("mwstate", saveState());
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e) {
  dragout = false;
  QMimeData const *data = e->mimeData();
  if (data->hasFormat("x-special/photohoard-versionid")) {
    dragout = true;
    e->accept();
  } else if (data->hasFormat("text/uri-list")) {
    QList<QUrl> urls = data->urls();
    if (!ImportGUI::acceptable(urls))
      return;
    dragin = true;
    e->accept();
  }
}

void MainWindow::dragLeaveEvent(QDragLeaveEvent *e) {
  if (dragout) {
    QPoint cp = mapFromGlobal(QCursor::pos());
    if (!rect().contains(cp))
      lightTable->ensureDragExportComplete();
    dragout = false;
  }
  e->accept();
}

void MainWindow::dragMoveEvent(QDragMoveEvent *e) {
  QMimeData const *data = e->mimeData();
  if (data->hasFormat("photohoard/versionid")) {
    e->setDropAction(Qt::IgnoreAction);
  }
}

void MainWindow::dropEvent(QDropEvent *e) {
  if (!dragin)
    return;

  //  Qt::DropAction act = e->dropAction();
  QMimeData const *data = e->mimeData();
  QList<QUrl> urls = data->urls();
  if (!ImportGUI::acceptable(urls))
    return;
  
  e->accept();
  auto *gui = new ImportGUI(db, scanner, urls, this);
  gui->showAndGo(true);

  dragin = false;
}

