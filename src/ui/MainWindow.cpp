// MainWindow.cpp

#include "MainWindow.h"
#include "DBInfoDialog.h"
#include "CreateDatabaseDialog.h"
#include "Version.h"
#include "LightTable.h"
#include "LayoutBar.h"
#include "FileBar.h"
#include "FilterBar.h"
#include "HelpBar.h"
#include "ColorLabelBar.h"
#include "Scanner.h"
#include "AutoCache.h"
#include "SessionDB.h"
#include "AllControls.h"
#include "HistoWidget.h"
#include <QDockWidget>
#include <QMenu>
#include "LiveAdjuster.h"
#include "MetaViewer.h"
#include "StatusBar.h"
#include "AppliedTagList.h"
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
  db(db), scanner(scanner), exporter(exporter), autocache(autocache) {
  QFileInfo dbf(db->photoDBFilename());
  setWindowTitle("Photohoard - " + dbf.baseName());
  
  
  adjuster = new LiveAdjuster(db, autocache, this);

  shortcutHelp = new ShortcutHelp();
  //  qDebug() << "help sizehint" << shortcutHelp->sizeHint();

  setCentralWidget(lighttable = new LightTable(db, autocache, adjuster,
                                               exporter, this));

  makeMenu();
  makeDocks();
  makeToolbars();

  connect(adjuster, SIGNAL(imageAvailable(Image16, quint64, QSize)),
          histogram, SLOT(setImage(Image16))); // is this ok?
  connect(adjuster, SIGNAL(imageAvailable(Image16, quint64, QSize)),
          metaViewer, SLOT(setImage(Image16, quint64)));

  connect(lighttable, SIGNAL(needImage(quint64, QSize)),
          autocache, SLOT(request(quint64, QSize)));
  connect(lighttable, SIGNAL(wantImage(quint64, QSize)),
          autocache, SLOT(requestIfEasy(quint64, QSize)));
  connect(lighttable, SIGNAL(recacheReoriented(QSet<quint64>)),
         autocache, SLOT(recache(QSet<quint64>)));
  connect(autocache, SIGNAL(available(quint64, Image16, quint64, QSize)),
          SLOT(updateImage(quint64, Image16, quint64, QSize)));
  if (scanner) {
    connect(scanner, SIGNAL(updatedBatch(QSet<quint64>)),
           lighttable, SLOT(rescan()));
    connect(scanner, SIGNAL(message(QString)),
            statusBar, SLOT(setMessage(QString)));
  }

 autocache->requestIfEasy(db->current(), QSize(1024, 1024));

  connect(lighttable, SIGNAL(newCurrent(quint64)),
          metaViewer, SLOT(setVersion(quint64)));
  connect(lighttable, SIGNAL(newZoom(double)),
          statusBar, SLOT(setZoom(double)));
  connect(lighttable, SIGNAL(newCollection(QString)),
          statusBar, SLOT(setCollection(QString)));

  connect(lighttable, SIGNAL(newCurrent(quint64)),
	  tagList, SLOT(setCurrent(quint64)));
  connect(lighttable, SIGNAL(newSelection()),
	  tagList, SLOT(newSelection()));

  connect(lighttable, SIGNAL(newCurrent(quint64)),
          histogram, SLOT(setVersion(quint64)));

  connect(lighttable, SIGNAL(newCurrent(quint64)),
	  allControls, SLOT(setVersion(quint64)));
  connect(allControls, SIGNAL(valuesChanged(quint64, int, Adjustments)),
	  adjuster, SLOT(reloadSliders(quint64, int, Adjustments)));
  connect(allControls, SIGNAL(maskChanged(quint64, int)),
	  adjuster, SLOT(reloadLayers(quint64, int)));
  connect(allControls, SIGNAL(layerSelected(quint64, int)),
	  lighttable, SLOT(visualizeLayer(quint64, int)));

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
	  lighttable, SLOT(updateFilterAndDialog()));

  dragout = false;
  dragin = false;
  setAcceptDrops(true);

  Settings s;
  if (s.contains("mwgeom"))
    restoreGeometry(s.get("mwgeom").toByteArray());
  if (s.contains("mwstate"))
    restoreState(s.get("mwstate").toByteArray());
  //  qDebug() << "Mainwindow" << geometry();
  lighttable->resize(size());
  lighttable->restoreSizes();
}

MainWindow::~MainWindow() {
}

void MainWindow::scrollToCurrent() {
  lighttable->scrollToCurrent();
}

void MainWindow::updateImage(quint64 i, Image16 img, quint64 chgid, QSize fs) {
  lighttable->updateImage(i, img, chgid, fs);
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
  lighttable->reloadVersion(vsn);
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
      lighttable->ensureDragExportComplete();
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
  gui->showAndGo();

  dragin = false;
}

void MainWindow::makeDocks() {
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
}

void MainWindow::makeToolbars() {
  constexpr Qt::ToolBarArea area = Qt::TopToolBarArea;
  addToolBar(area, fileBar = new FileBar(db, autocache,
                                         exporter, scanner, this));
  addToolBar(area, layoutBar = new LayoutBar(lighttable, this));
  if (!db->isReadOnly())
    addToolBar(area, colorLabelBar = new ColorLabelBar(db, lighttable, this));
  addToolBar(area, filterBar = new FilterBar(lighttable, menu, this));
  //  addToolBarBreak(area);
  //  addToolBar(area, helpBar = new HelpBar(this));
  //  addToolBar(area, databaseBar = new DatabaseBar(db, this));
  addHiddenActions();

  shortcutHelp->addSection("General", fileBar->actions());
  shortcutHelp->addSection("General", filterBar->actions());
  //  shortcutHelp->addSection("General", databaseBar->actions());
  shortcutHelp->addSection("Layout", layoutBar->actions());
  if (!db->isReadOnly())
    shortcutHelp->addSection("Labels and marks", colorLabelBar->actions());
  shortcutHelp->addSection("Image strip and photo editor",
                           lighttable->actions());
  if (!db->isReadOnly())
    shortcutHelp->addSection("Slider panel", allControls->actions());
}

void MainWindow::addHiddenActions() {
  auto add = [this](Action const &act) {
     hiddenactions << act;
     addAction(new PAction{hiddenactions.last(), this});
  };

  add(Action{Qt::CTRL + Qt::SHIFT + Qt::Key_A,
             "Clear selection", 
             [=]() { lighttable->clearSelection(); }});
  add(Action{Qt::CTRL + Qt::Key_A,
             "Select all", 
             [=]() { lighttable->selectAll(); }});
  shortcutHelp->addSection("General", hiddenactions);
}

void MainWindow::makeMenu() {
  menu = new QMenu(this);
  auto add = [this](Action const &act) {
    PAction *pact = new PAction{act, this};
    addAction(pact);
    menu->addAction(pact);
  };

  add(Action{Qt::CTRL + Qt::SHIFT + Qt::Key_N, "&New database…",
             [this]() { newDatabase(); }});
  add(Action{Qt::CTRL + Qt::SHIFT + Qt::Key_O, "&Open other database…",
             [this]() { openOther(); }});
  menu->addSeparator();
  add(Action{Qt::CTRL + Qt::SHIFT + Qt::Key_B, "Database &info…",
             [this]() { databaseInfo(); }});
  add(Action{0, "&About Photohoard…",
             [this]() { showAbout(); }});
  add(Action{Qt::CTRL + Qt::Key_H, "Keyboard &help…",
             [this]() { showShortcutHelp(); }});
  add(Action{Qt::CTRL + Qt::Key_Q, "&Quit",
             []() { QApplication::quit(); }});
}

void MainWindow::showAbout() {
  QString me = "<b>Photohoard</b>";
  QString vsn = Version::toString();
  QMessageBox::about(0, "About Photohoard",
                     me + " " + vsn
                     + "<p>" + "(C) 2016–2023 Daniel A. Wagenaar\n"
                     + "<p>" + me + " is a program to organize collections of digital photographs. More information is available at <a href=\"http://www.github.com/wagenadl/photohoard\">github.com/wagenadl/photohoard</a>.\n"
                     + "<p>" + me + " is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.\n"
                     + "<p>" + me + " is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.\n"
                     + "<p>" + "You should have received a copy of the GNU General Public License along with this program. If not, see <a href=\"http://www.gnu.org/licenses/gpl-3.0.en.html\">www.gnu.org/licenses/gpl-3.0.en.html</a>.");
}

void MainWindow::databaseInfo() {
  auto *dlg = new DBInfoDialog(db, this);
  dlg->exec();
  delete dlg;
}

void MainWindow::newDatabase() {
  auto *dlg = new CreateDatabaseDialog(this);
  dlg->exec();
  delete dlg;
}

void MainWindow::openOther() {
  qDebug() << "open other";
}
