// LightTable.cpp

#include "LightTable.h"
#include "StripView.h"
#include "SlideView.h"
#include "Datestrip.h"
#include "PDebug.h"
#include <QScrollBar>
#include "Selection.h"
#include "Slide.h"
#include "StripScene.h"
#include "Exif.h"
#include "FilterDialog.h"
#include <QKeyEvent>
#include "PurgeDialog.h"
#include <QUrl>
#include "DragOut.h"
#include "MultiDragDialog.h"
#include "Adjuster.h"
#include <QMimeData>
#include <QDrag>
#include "Settings.h"

LightTable::LightTable(SessionDB *db, AutoCache *cache,
                       LiveAdjuster *adj, Exporter *expo,
                       QWidget *parent):
  QSplitter(parent), db(db), cache(cache),
  exporter(expo),
  adjuster(adj) {
  setObjectName("LightTable");
  lay=lastlay=LayoutBar::Layout::VGrid;
  showmax = false;
  
  bool oldcrash = db->simpleQuery("select count(*) from starting").toInt()>0;
  if (oldcrash) {
    Untransaction t(db);
    db->setCurrent(0);
    db->query("delete from expanded");
    db->query("delete from filtersettings");
  }
  { Untransaction t(db);
    db->query("insert into starting values(1)");
  }

  dragout = 0;
  filterDialog = new FilterDialog(db);
  applyFilterSettings();

  selection = new Selection(db);

  strips = new StripView(db);

  addWidget(strips);

  slide = new SlideView(db);
  addWidget(slide);

  setStretchFactor(0, 0);
  setStretchFactor(1, 100);
  restoreSizes();
  setLayout(lay);

  connect(strips, SIGNAL(pressed(quint64,
                                 Qt::MouseButton, Qt::KeyboardModifiers)),
          this, SLOT(slidePress(quint64,
                                Qt::MouseButton, Qt::KeyboardModifiers)));
  connect(strips, SIGNAL(needImage(quint64, QSize)),
	  this, SIGNAL(needImage(quint64, QSize)));
  connect(strips, SIGNAL(dragStarted(quint64)),
          this, SLOT(startDrag(quint64)));
  connect(strips, SIGNAL(idealSizeChanged()), SLOT(resizeStrip()));
  connect(this, SIGNAL(splitterMoved(int, int)),
	  SLOT(saveSplitterPos()));
  connect(strips->scene(),
          SIGNAL(pressed(Qt::MouseButton, Qt::KeyboardModifiers)),
          this, SLOT(bgPress(Qt::MouseButton, Qt::KeyboardModifiers)));

  connect(slide, SIGNAL(needImage(quint64, QSize)),
	  this, SIGNAL(wantImage(quint64, QSize)));
  connect(slide, SIGNAL(needImage(quint64, QSize)),
          adjuster, SLOT(requestAdjusted(quint64, QSize)));
  connect(slide, SIGNAL(newSize(QSize)),
          this, SIGNAL(newSlideSize(QSize)));
  connect(slide, SIGNAL(newZoom(double)),
          this, SIGNAL(newZoom(double)));

  connect(adjuster, SIGNAL(imageAvailable(Image16, quint64, QSize)),
          SLOT(updateAdjusted(Image16, quint64, QSize)));

  connect(filterDialog, SIGNAL(applied()),
	  SLOT(updateFilter()));
  
  quint64 c = db->current();
  if (c) {
    selectNearestInFilter(c);
    Selection(db).add(c);
  }

  { Untransaction t(db);
    db->query("delete from starting");
  }

  makeActions();
}

void LightTable::restoreSizes() {
  lastgridsize = Settings()
    .get("gridsize", strips->idealSize(Strip::Arrangement::Grid))
    .toInt();
  setSizes(QList<int>() << lastgridsize << width()-lastgridsize);
  qDebug() <<"restoresizes" << lastgridsize << width()-lastgridsize;
}  

LightTable::~LightTable() {
  delete filterDialog;
  delete selection;
}

void LightTable::ensureReasonableGridSize() {
  int s0 = sizes()[0];
  int s1 = strips->idealSize(Strip::Arrangement::Grid);
  if (s0<s1) {
    lastgridsize = s1;
    setSizes(QList<int>() << lastgridsize
             << ((orientation()==Qt::Vertical)?height():width())-lastgridsize);
  }
}


void LightTable::setLayout(LayoutBar::Layout act) {
  lastlay = lay;
  switch (act) {
  case LayoutBar::Layout::FullGrid:
    strips->setArrangement(Strip::Arrangement::Grid);
    strips->show();
    slide->hide();
    lay = act;
    break;
  case LayoutBar::Layout::HGrid:
    strips->setArrangement(Strip::Arrangement::Grid);
    setOrientation(Qt::Vertical);
    if (lay==LayoutBar::Layout::HLine
        || lay==LayoutBar::Layout::VLine
        || lay==LayoutBar::Layout::FullPhoto)
      setSizes(QList<int>() << lastgridsize << height()-lastgridsize);
    ensureReasonableGridSize();
    strips->show();
    ensureSlideShown();
    lay = act;
    break;
  case LayoutBar::Layout::VGrid:
    strips->setArrangement(Strip::Arrangement::Grid);
    setOrientation(Qt::Horizontal);
    if (lay==LayoutBar::Layout::HLine
        || lay==LayoutBar::Layout::VLine
        || lay==LayoutBar::Layout::FullPhoto)
      setSizes(QList<int>() << lastgridsize << width()-lastgridsize);
    ensureReasonableGridSize();
    strips->show();
    ensureSlideShown();
    lay = act;
    break;
  case LayoutBar::Layout::HLine: {
    if (lay==LayoutBar::Layout::HGrid
        || lay==LayoutBar::Layout::HGrid)
      lastgridsize = sizes()[0];
    int ts = strips->tileSize();
    setOrientation(Qt::Vertical);
    strips->show();
    strips->setArrangement(Strip::Arrangement::Horizontal);
    strips->setTileSize(ts);
    setSizes(QList<int>() << strips->idealSize(Strip::Arrangement::Horizontal)
             << height());
    ensureSlideShown();
    lay = act;
  } break;
  case LayoutBar::Layout::VLine: {
    if (lay==LayoutBar::Layout::HGrid
        || lay==LayoutBar::Layout::VGrid)
      lastgridsize = sizes()[0];
    int ts = strips->tileSize();
    setOrientation(Qt::Horizontal);
    strips->show();
    strips->setArrangement(Strip::Arrangement::Vertical);
    strips->setTileSize(ts);
    setSizes(QList<int>() << strips->idealSize(Strip::Arrangement::Vertical)
             << width());
    ensureSlideShown();
    lay = act;
  } break;
  case LayoutBar::Layout::FullPhoto:
    if (lay==LayoutBar::Layout::HGrid
        || lay==LayoutBar::Layout::VGrid)
      lastgridsize = sizes()[0];
    strips->hide();
    ensureSlideShown();
    lay = act;
    break;
  case LayoutBar::Layout::Line:
    if (orientation()==Qt::Vertical)
      setLayout(LayoutBar::Layout::VLine);
    else
      setLayout(LayoutBar::Layout::HLine);
    return;
  case LayoutBar::Layout::HalfGrid:
    if (orientation()==Qt::Vertical)
      setLayout(LayoutBar::Layout::VGrid);
    else
      setLayout(LayoutBar::Layout::HGrid);
    return;
  case LayoutBar::Layout::ToggleFullPhoto:
    if (lay==LayoutBar::Layout::FullPhoto)
      setLayout(lastlay);
    else
      setLayout(LayoutBar::Layout::FullPhoto);
    return;
  case LayoutBar::Layout::ToggleFullScreen:
    COMPLAIN("ToggleFullScreen NYI");
    break;
  case LayoutBar::Layout::ToggleOrg:
    strips->toggleOrganization();
    break;
  }
  if (lastlay==LayoutBar::Layout::FullGrid && lay!=lastlay)
    emit needImage(db->current(), displaySize());
  if (lay!=LayoutBar::Layout::FullPhoto)
    scrollToCurrent();
}

void LightTable::ensureSlideShown() {
  slide->show();
  if (slide->currentVersion() != db->current())
    updateMainSlide();
}  

void LightTable::slidePress(quint64 i, Qt::MouseButton b,
                            Qt::KeyboardModifiers mm) {
  switch (b) {
  case Qt::LeftButton:
    select(i, mm);
    break;
  default:
    break;
  }
}

void LightTable::toggleSelection(quint64 i) {
  // Control: Toggle whether image i is selected
  if (selection->contains(i)) {
    selection->remove(i);
    if (i==db->current())
      makeCurrent(0);
    else
      updateSlide(i);
  } else {
    selection->add(i);
    if (db->current()==0)
      makeCurrent(i);
    else
      updateSlide(i);
  }
  emit newSelection();
}

void LightTable::extendOrShrinkSelection(quint64 i) {
  // Shift: Select a range from current id to new i
  // Currently, this always adds, but that needs not be the case
  // Also, this should work differently for folder-tree organization

  quint64 cur = db->current();
  if (cur==0) {
    makeCurrent(i);
    cur = i;
  }
  
  switch (strips->organization()) {
  case Strip::Organization::ByDate: {
    QDateTime a = db->captureDate(db->photoFromVersion(cur));
    QDateTime b = db->captureDate(db->photoFromVersion(i));
    if (a>b)
      selection->addDateRange(b, a);
    else
      selection->addDateRange(a, b);
  } break;
  case Strip::Organization::ByFolder: {
    PhotoDB::PhotoRecord a = db->photoRecord(db->photoFromVersion(cur));
    PhotoDB::PhotoRecord b = db->photoRecord(db->photoFromVersion(i));

    if (a.folderid==b.folderid) {
      // easy case
      if (a.capturedate>b.capturedate)
        selection->addDateRange(b.capturedate, a.capturedate);
      else
        selection->addDateRange(a.capturedate, b.capturedate);
    } else {
      if (db->folder(a.folderid)<db->folder(b.folderid)) {
        selection->addRestOfFolder(a.folderid, a.capturedate);
        selection->addFoldersBetween(a.folderid, b.folderid);
        selection->addStartOfFolder(b.folderid, b.capturedate);
      } else {
        selection->addRestOfFolder(b.folderid, b.capturedate);
        selection->addFoldersBetween(b.folderid, a.folderid);
        selection->addStartOfFolder(a.folderid, a.capturedate);
      }
    }
  } break;
  }
  emit newSelection();
  strips->scene()->update();
}  

void LightTable::simpleSelection(quint64 i, bool keep) {
  if (keep && i==db->current())
    return;

  if (i>0 && !(selection->contains(i) && keep)) {
    // clear selection
    bool localupdate = true; // if we have just a few in selection, we'll
    // repaint just those slides, otherwise the whole view
    if (selection->count()<=10) {
      QSet<quint64> ss = selection->current();
      selection->clear();
      for (auto k: ss) {
        Slide *s = strips->strip()->slideByVersion(k);
        if (s)
          s->update();
      }
    } else {
      localupdate = false;
      selection->clear();
    }
    selection->add(i);
    emit newSelection();
    if (!localupdate)
      strips->scene()->update();
  }

  makeCurrent(i);
}

void LightTable::makeCurrent(quint64 i) {
  quint64 oldcur = db->current();
  if (i==oldcur)
    return;

  db->setCurrent(i);
  emit newCurrent(i);

  updateSlide(oldcur);
  updateSlide(i);
  strips->scrollIfNeeded();
  
  updateMainSlide();
}

void LightTable::updateMainSlide() {
  int cur = db->current();
  if (cur>0 && slide->isVisibleTo(this)) {
    QSize osize = db->originalSize(cur);
    QSize ns = Adjuster::mapCropSize(osize, Adjustments::fromDB(cur, *db));
    adjuster->markVersionAndSize(cur, ns);
    slide->newImage(cur, ns);
    emit newZoom(slide->currentZoom());
    // emit needImage(cur, displaySize()); // IS THIS NEEDED??
  } else {
    adjuster->clear();
    slide->clear();
  }
}

void LightTable::select(quint64 i, Qt::KeyboardModifiers m) {
  if (m & Qt::ControlModifier) {
    toggleSelection(i);
   } else if (m & Qt::ShiftModifier) {
    extendOrShrinkSelection(i);
  } else {
    simpleSelection(i, true);
  }
}    

void LightTable::updateSlide(quint64 i) {
  Slide *s = strips->strip()->slideByVersion(i);
  if (s)
    s->update();
}

PSize LightTable::displaySize() const {
  if (slide->isVisible())
    return slide->desiredSize();
  else
    return PSize::square(strips->strip()->tileSize());
}

void LightTable::updateAdjusted(Image16 img, quint64 i, QSize fs) {
  if (img.isNull())
    return;
  strips->updateImage(i, img, false);
  slide->updateImage(i, img, false, fs);
}

void LightTable::updateImage(quint64 i, Image16 img, quint64 chgid, QSize fs) {
  strips->updateImage(i, img, chgid>0);
  slide->updateImage(i, img, chgid>0, fs);
}

void LightTable::rescan(bool rebuildFilter) {
  if (rebuildFilter)
    applyFilterSettings();
  strips->rescan();
}

void LightTable::updateSelectedTiles() {
  if (selection->count() > 10) {
    strips->scene()->update();
  } else {
    QSet<quint64> cc = Selection(db).current();
    for (auto vsn: cc) {
      Slide *s = strips->strip()->slideByVersion(vsn);
      if (s)
	s->update();
    }
  }
}

void LightTable::resizeStrip() {
  if (lay==LayoutBar::Layout::HLine)
    setSizes(QList<int>() << strips->idealSize() << height());
  else if (lay==LayoutBar::Layout::VLine) 
    setSizes(QList<int>() << strips->idealSize() << width());
  else
    return;
  //  scrollToCurrent();
}

void LightTable::increaseTileSize(double factor) {
  strips->setTileSize(factor*strips->tileSize());
  Settings().set("tilesize", strips->tileSize());
}

void LightTable::openFilterDialog() {
  filterDialog->show();
}

void LightTable::selectAll() {
  selection->selectAll();
  strips->scene()->update();
}

void LightTable::clearSelection() {
  quint64 cur = db->current();
  if (cur) {
    simpleSelection(cur, false);
  } else {
    if (selection->count()<=10) {
      QSet<quint64> ss = selection->current();
      selection->clear();
      for (auto i: ss) {
        Slide *s = strips->strip()->slideByVersion(i);
        if (s)
          s->update();
      }
    } else {
      selection->clear();
      strips->scene()->update();
    }
  }
}  

void LightTable::bgPress(Qt::MouseButton b, Qt::KeyboardModifiers m) {
  switch (b) {
  case Qt::LeftButton:
    if (m==Qt::NoModifier)
      clearSelection();
    break;
  default:
    break;
  }
}

void LightTable::scrollToCurrent() {
  strips->scrollToCurrent();
}

void LightTable::updateFilterAndDialog() {
  filterDialog->populate();
  updateFilter();
}

void LightTable::updateFilter() {
  int oldcur = db->current();
  select(0);
  rescan(true);
  selectNearestInFilter(oldcur);
  scrollToCurrent();
}

void LightTable::selectNearestInFilter(quint64 vsn) {
  QSqlQuery q = db->query("select version, abs(version-:a) as x from filter"
                          " order by x limit 1", vsn);
  if (q.next())
    select(q.value(0).toULongLong());
  else
    select(0);
}

void LightTable::applyFilterSettings() {
  // was populatefilterfromdialog
  Filter f(db);
  f.loadFromDb();
  Untransaction t(db);
  db->query("delete from filter");
  db->query("insert into filter select versions.id, photos.id from versions "
           + f.joinClause() + " where " + f.whereClause());
  db->query("delete from selection"
           " where version not in (select version from filter)");
  emit newCollection(f.hasCollection() ? f.collection() : QString(""));
}

void LightTable::rotateSelected(int dphi) {
  QSet<quint64> vsns = Selection(db).current();

  for (auto id: vsns)
    strips->quickRotate(id, dphi);

  quint64 oldcur = db->current();
  if (vsns.contains(oldcur)) {
    /* Somehow update the slideview, which involves updating the
       liveadjuster, which is not trivial, at least not when I am
       tired.  In fact, it really is tricky, because the live
       adjuster, or rather its originalfinder or interruptableadjuster
       may be busy. And they can signal at any time, because they run
       in a different thread.  The current implementation is really
       lame and causes a lot of flashing.
     */
    makeCurrent(0);
  }  
  
  Transaction t(db);
  for (auto id: vsns) {
    int orient
      = db->simpleQuery("select orient from versions where id==:a", id).toInt();
    orient = (orient + dphi) & 3;
    db->query("update versions set orient=:a where id==:b", orient, id);
  }
  t.commit();

  // and now rescan
  emit recacheReoriented(vsns);

  if (db->current()!=oldcur)
    makeCurrent(oldcur);

}

Actions const &LightTable::actions() const {
  ASSERT(slide);
  static Actions x = acts + slide->actions();
  return x;
}

void LightTable::makeActions() {
  acts
  << Action{ { Qt::Key_Minus, Qt::Key_Underscore }, "Zoom out (try Shift, Alt)",
      [&]() { slide->changeZoomLevel(QPoint(), -0.5, true); }}
  << Action{ { Qt::Key_Minus | Qt::ShiftModifier,
           Qt::Key_Underscore | Qt::ShiftModifier}, "",
      [&]() { slide->changeZoomLevel(QPoint(), -1, true); }}
  << Action{ { Qt::Key_Minus | Qt::AltModifier,
           Qt::Key_Underscore | Qt::AltModifier}, "",
      [&]() { slide->changeZoomLevel(QPoint(), -0.125, true); }}
  << Action{ { Qt::Key_Plus, Qt::Key_Equal }, "Zoom in (try Shift, Alt)",
      [&]() { slide->changeZoomLevel(QPoint(), 0.5, true); }}
  << Action{ { Qt::Key_Plus | Qt::ShiftModifier,
           Qt::Key_Equal | Qt::ShiftModifier}, "",
      [&]() { slide->changeZoomLevel(QPoint(), 1, true); }}
  << Action{ { Qt::Key_Plus | Qt::AltModifier,
           Qt::Key_Equal | Qt::AltModifier}, "",
      [&]() { slide->changeZoomLevel(QPoint(), 0.125, true); }}
  << Action{ Qt::Key_1, "Zoom 1:1",
         [&]() { slide->setZoom(1); }}
  << Action{ Qt::Key_0, "Scale to fit",
         [&]() { slide->scaleToFit(); }}
  << Action{ "Arrows", "Navigate between images" }
  << Action{ Qt::Key_Up, "",
      [&]() {
      quint64 v = strips->strip()->versionAbove(db->current());
      if (v)
        slidePress(v, Qt::LeftButton, 0);
    }}
  << Action{ Qt::Key_Down, "",
      [&]() {
      quint64 v = strips->strip()->versionBelow(db->current());
      if (v)
        slidePress(v, Qt::LeftButton, 0);
    }}
  << Action{ Qt::Key_Left, "",
      [&]() {
      quint64 v = strips->strip()->versionLeftOf(db->current());
      if (v)
        slidePress(v, Qt::LeftButton, 0);
    }}
  << Action{ Qt::Key_Right, "",
      [&]() {
      quint64 v = strips->strip()->versionRightOf(db->current());
      if (v)
        slidePress(v, Qt::LeftButton, 0);
    }}
  << Action { Qt::Key_Period, "Center on current image",
      [&]() {
      quint64 v = db->current();
      db->setCurrent(0); // trick
      if (v)
        slidePress(v, Qt::LeftButton, 0);
    }}         
  << Action { Qt::Key_BracketLeft, "Select previous image",
      [&]() {
      quint64 v = strips->strip()->versionBefore(db->current());
      if (v)
        slidePress(v, Qt::LeftButton, 0);
    }}
  << Action { Qt::Key_BracketRight, "Select next image",
      [&]() {
      quint64 v = strips->strip()->versionAfter(db->current());
      if (v)
        slidePress(v, Qt::LeftButton, 0);
    }}
  << Action { Qt::CTRL + Qt::Key_N, "New version from current",
         [&]() {
      db->newVersion(db->current(), true);
      rescan();
    }}
  << Action { Qt::CTRL + Qt::SHIFT + Qt::Key_N, "New version from original",
         [&]() {
      db->newVersion(db->current(), false);
      rescan();
    }}
  << Action { Qt::CTRL + Qt::SHIFT + Qt::Key_X, "Purge rejects",
         [&]() {
      PurgeDialog::purgeDialog(db, cache);
      rescan();
    }}
  ;
}

void LightTable::keyPressEvent(QKeyEvent *e) {
  if (acts.activateIf(e)) {
    e->accept();
    return;
  }
}

void LightTable::reloadVersion(quint64 vsn) {
  if (vsn==db->current()) {
    makeCurrent(0);
    makeCurrent(vsn);
  }
  Slide *slide = strips->strip()->slideByVersion(vsn);
  if (slide)
    slide->reload();
}

void LightTable::startDrag(quint64 id) {
  qDebug() << "Start drag for " << id;

  PhotoDB::PhotoRecord pr = db->photoRecord(db->photoFromVersion(id));
  QString fn = pr.filename;
  qDebug() << "  fn = " << fn;
  if (fn.isEmpty()) {
    COMPLAIN("fn is empty-can't drag");
    return;
  }

  int idx = fn.lastIndexOf(".");
  if (idx>=0)
    fn = fn.left(idx) + ".jpg";
  else
    fn += ".jpg";
  
  fn = "/tmp/" + fn;

  QMimeData *data = new QMimeData;
  QByteArray id_ar(reinterpret_cast<const char*>(&id), sizeof(id));
  data->setData("x-special/photohoard-versionid", id_ar);
  QList<QUrl> lst; lst << QUrl("file://" + fn);
  data->setUrls(lst);

  QDrag *drag = new QDrag(this);
  drag->setMimeData(data);

  dragout = new DragOut(db, exporter, id, fn, this);

  Qt::DropAction act = drag->exec(Qt::MoveAction);
  if (act>0)
    dragout->finish();
  else
    dragout->cancel();

  delete dragout;
  dragout = 0;

  if (act==0)
    return;

  if (!MultiDragDialog::shouldNotShow()) {
    QSet<quint64> cursel = selection->current();
    cursel.remove(id);
    if (!cursel.isEmpty()) {
      auto *mdd = new MultiDragDialog(db, exporter, cursel);
      mdd->show();
    }
  }
}

void LightTable::ensureDragExportComplete() {
  if (dragout)
    dragout->ensureComplete();
}

void LightTable::visualizeLayer(quint64 vsn, int lay) {
  slide->visualizeLayer(vsn, lay);
}

void LightTable::saveSplitterPos() {
  switch (lay) {
  case LayoutBar::Layout::HGrid:
    qDebug() << "saveSplitterPos H" << strips->width() << strips->height();
    Settings().set("gridsize", strips->height());
    break;
  case LayoutBar::Layout::VGrid:
    qDebug() << "saveSplitterPos V" << strips->width() << strips->height();
    Settings().set("gridsize", strips->width());
    break;
  default:
    break;
  }
}

