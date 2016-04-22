// MainWindow.cpp

#include "MainWindow.h"
#include "LightTable.h"
#include "LayoutBar.h"
#include "FileBar.h"
#include "FilterBar.h"
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


MainWindow::MainWindow(SessionDB *db,
                       Scanner *scanner, AutoCache *autocache,
                       Exporter *exporter):
  db(db), scanner(scanner), exporter(exporter) {

  setWindowTitle("Photohoard");
  
  QDockWidget *dock = new QDockWidget("Histogram", this);
  dock->setWidget(histogram = new HistoWidget(this));
  dock->setTitleBarWidget(new QWidget());
  addDockWidget(Qt::RightDockWidgetArea, dock);
  
  dock = new QDockWidget("Adjustments", this);
  dock->setWidget(allControls = new AllControls(db->isReadOnly(), this));
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
  dock->setWidget(statusBar = new StatusBar(db, this));
  dock->setTitleBarWidget(new QWidget());
  addDockWidget(Qt::RightDockWidgetArea, dock);

  adjuster = new LiveAdjuster(db, allControls, autocache, this);

  shortcutHelp = new ShortcutHelp();
  
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

  shortcutHelp->addSection("General", fileBar->actions());
  shortcutHelp->addSection("General", filterBar->actions());
  shortcutHelp->addSection("Layout", layoutBar->actions());
  if (!db->isReadOnly())
    shortcutHelp->addSection("Labels and marks", colorLabelBar->actions());
  shortcutHelp->addSection("Image strip and photo editor",
                           lightTable->actions());
  if (!db->isReadOnly())
    shortcutHelp->addSection("Slider panel", allControls->actions());
  
  connect(adjuster, SIGNAL(imageAvailable(Image16, quint64)),
          histogram, SLOT(setImage(Image16))); // is this ok?
  connect(adjuster, SIGNAL(imageAvailable(Image16, quint64)),
          metaViewer, SLOT(setImage(Image16, quint64)));

  connect(lightTable, SIGNAL(needImage(quint64, QSize)),
          autocache, SLOT(request(quint64, QSize)));
  connect(lightTable, SIGNAL(wantImage(quint64, QSize)),
          autocache, SLOT(requestIfEasy(quint64, QSize)));
  connect(lightTable, SIGNAL(recacheReoriented(QSet<quint64>)),
	  autocache, SLOT(recache(QSet<quint64>)));
  connect(autocache, SIGNAL(available(quint64, Image16, quint64)),
          SLOT(updateImage(quint64, Image16, quint64)));
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

  connect(exporter, SIGNAL(completed(QString, int, int)),
          SLOT(reportExportResults(QString, int, int)));
  
  tagList->setCurrent(db->current());
  metaViewer->setVersion(db->current());
  if (lightTable->filter().hasCollection())
    statusBar->setCollection(lightTable->filter().collection());

  connect(fileBar->sliderClipboard(), SIGNAL(modified(quint64)),
          SLOT(reloadVersion(quint64)));

  dragout = false;
  dragin = false;
  setAcceptDrops(true);
}

MainWindow::~MainWindow() {
}

void MainWindow::scrollToCurrent() {
  lightTable->scrollToCurrent();
}

void MainWindow::updateImage(quint64 i, Image16 img, quint64 chgid) {
  lightTable->updateImage(i, img, chgid);
  if (i==db->current())
    histogram->setImage(img); // this is *bad*
}

void MainWindow::showShortcutHelp() {
  shortcutHelp->show();
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
  QApplication::quit();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e) {
  dragout = false;
  QMimeData const *data = e->mimeData();
  if (data->hasFormat("x-special/photohoard-versionid")) {
    dragout = true;
    e->accept();
  } else if (data->hasFormat("text/uri-list")) {
    QList<QUrl> urls = data->urls();
    for (auto const &url: urls)
      if (!url.isLocalFile())
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

  Qt::DropAction act = e->dropAction();
  QMimeData const *data = e->mimeData();
  QList<QUrl> urls = data->urls();

  QString coll = "";
  {
    Filter filter(db);
    filter.loadFromDb();
    if (filter.hasCollection())
      coll = filter.collection();
  }
  
  qDebug() << "Drop action" << act;
  if (coll.isEmpty())
    qDebug() << "  Ask for drop collection!";

  //  scanner->importDragged(urls, coll);

  dragin = false;
}

