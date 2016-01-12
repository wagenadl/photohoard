// LightTable.cpp

#include "LightTable.h"
#include "NoResult.h"
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

LightTable::LightTable(PhotoDB *db, LiveAdjuster *adj, QWidget *parent):
  QSplitter(parent), db(db), adjuster(adj) {
  setObjectName("LightTable");
  curr = 0;
  lay=lastlay=LayoutBar::Layout::VGrid;
  showmax = false;
  
  bool oldcrash = db->simpleQuery("select count(*) from starting").toInt()>0;
  pDebug() << "Hello world";
  if (oldcrash) {
    Untransaction t(db);
    db->query("update current set version=null");
    db->query("delete from expanded");
    db->query("delete from filtersettings");
  }
  pDebug() << "Starting";
  { Untransaction t(db);
    db->query("insert into starting values(1)");
  }
  pDebug() << "Hello world";

  filterDialog = new FilterDialog(db);
  populateFilterFromDialog();
  
  selection = new Selection(db);

  strips = new StripView(db);

  addWidget(strips);

  slide = new SlideView();
  addWidget(slide);

  lastgridsize = strips->idealSize(Strip::Arrangement::Grid);
  setStretchFactor(0, 0);
  setStretchFactor(1, 100);
  setSizes(QList<int>() << lastgridsize << width()-lastgridsize);
  setLayout(lay);

  connect(strips, SIGNAL(pressed(quint64,
                                 Qt::MouseButton, Qt::KeyboardModifiers)),
          this, SLOT(slidePress(quint64,
                                Qt::MouseButton, Qt::KeyboardModifiers)));
  connect(strips, SIGNAL(needImage(quint64, QSize)),
	  this, SIGNAL(needImage(quint64, QSize)));
  connect(strips, SIGNAL(idealSizeChanged()), SLOT(resizeStrip()));
  connect(strips->scene(),
          SIGNAL(pressed(Qt::MouseButton, Qt::KeyboardModifiers)),
          this, SLOT(bgPress(Qt::MouseButton, Qt::KeyboardModifiers)));

  connect(slide, SIGNAL(needLargerImage()),
	  this, SLOT(requestLargerImage()));
  connect(slide, SIGNAL(newSize(QSize)),
          this, SIGNAL(newSlideSize(QSize)));
  connect(slide, SIGNAL(newSize(QSize)),
          this, SLOT(requestLargerImage()));
  connect(slide, SIGNAL(newZoom(double)),
          this, SIGNAL(newZoom(double)));

  connect(adjuster, SIGNAL(imageChanged(Image16, quint64)),
          SLOT(updateAdjusted(Image16, quint64)));

  connect(filterDialog, SIGNAL(apply()),
	  SLOT(applyFilterFromDialog()));
  
  quint64 c = db->simpleQuery("select * from current").toULongLong();
  if (c)
    select(c);

  { Untransaction t(db);
    db->query("delete from starting");
  }

  makeActions();
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
    slide->show();
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
    slide->show();
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
    slide->show();
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
    slide->show();
    lay = act;
  } break;
  case LayoutBar::Layout::FullPhoto:
    if (lay==LayoutBar::Layout::HGrid
        || lay==LayoutBar::Layout::VGrid)
      lastgridsize = sizes()[0];
    strips->hide();
    slide->show();
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
    pDebug() << "ToggleFullScreen NYI";
    break;
  case LayoutBar::Layout::ToggleOrg:
    strips->toggleOrganization();
    break;
  }
  if (lastlay==LayoutBar::Layout::FullGrid && lay!=lastlay) {
    emit needImage(curr, displaySize());
    requestLargerImage();
  }
  if (lay!=LayoutBar::Layout::FullPhoto)
    scrollToCurrent();
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
    if (i==curr)
      makeCurrent(0);
    else
      updateSlide(i);
  } else {
    selection->add(i);
    if (curr==0)
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

  if (curr==0)
    makeCurrent(i);
  pDebug() << "extendorshrink" << i << int(strips->organization());

  switch (strips->organization()) {
  case Strip::Organization::ByDate: {
    QDateTime a = db->captureDate(db->photoFromVersion(curr));
    QDateTime b = db->captureDate(db->photoFromVersion(i));
    if (a>b)
      selection->addDateRange(b, a);
    else
      selection->addDateRange(a, b);
  } break;
  case Strip::Organization::ByFolder: {
    PhotoDB::PhotoRecord a = db->photoRecord(db->photoFromVersion(curr));
    PhotoDB::PhotoRecord b = db->photoRecord(db->photoFromVersion(i));
    pDebug() << curr << i << a.folderid << b.folderid << a.capturedate << b.capturedate;
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
  // Ignore other modifiers for the moment
  if (keep && i==curr)
    return;

  if (i>0 && (!selection->contains(i) || !keep)) {
    bool localupdate = true; // if we have just a few in selection, we'll
    // repaint just those slides, otherwise the whole view
    if (selection->count()<=10) {
      QSet<quint64> ss = selection->current();
      selection->clear();
      for (auto i: ss) {
        Slide *s = strips->strip()->slideByVersion(i);
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
  if (i==curr)
    return;

  if (i>0)
    db->query("update current set version=:a", i);
  else
    db->query("update current set version=null");
  
  updateSlide(curr);
  updateSlide(i);

  if (i>0) {
    QSqlQuery q = db->query("select width, height, orient"
			    " from versions"
			    " inner join photos on versions.photo==photos.id"
			    " where versions.id==:a", i);
    ASSERT(q.next());
    int w = q.value(0).toInt();
    int h =  q.value(1).toInt();
    Exif::Orientation ori = Exif::Orientation(q.value(2).toInt());
    q.finish();
    QSize ns = (ori==Exif::CW || ori==Exif::CCW) ? QSize(h, w): QSize(w, h);
    slide->newImage(i, ns);
  } else {
    slide->clear();
    adjuster->clear();
  }

  curr = i;
  emit newCurrent(curr);
  emit newZoom(slide->currentZoom());
  if (curr>0 && lay!=LayoutBar::Layout::FullGrid) {
    pDebug() << "emitting needimage " << curr;
    emit needImage(curr, displaySize());
    pDebug() << "emitted needimage " << curr;
    // Check that this works
    requestLargerImage();
  }

  strips->scrollIfNeeded();
}

void LightTable::select(quint64 i, Qt::KeyboardModifiers m) {
  pDebug() << "select" << i;
  //  Database::enableDebug();
  if (m & Qt::ControlModifier) {
    toggleSelection(i);
   } else if (m & Qt::ShiftModifier) {
    extendOrShrinkSelection(i);
  } else {
    simpleSelection(i, true);
  }
  pDebug() << "select done" << i;
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

void LightTable::requestLargerImage() {
  if (slide->isVisible()) {
    // also request from cache if helpful and readily available?
    adjuster->requestAdjusted(curr, slide->desiredSize());
  } else {
    adjuster->markVersionAndSize(curr, slide->desiredSize());
  }
}

void LightTable::updateAdjusted(Image16 img, quint64 i) {
  if (i==curr)
    pDebug() << "LightTable::updateAdjusted current" << i << img.size();
  if (img.isNull())
    return;
  strips->updateImage(i, img, true);

  if (i==curr)
    slide->updateImage(i, img, true);
}

void LightTable::updateImage(quint64 i, Image16 img, quint64 chgid) {
  if (i==curr)
    pDebug() << "LightTable::updateImage current" << i << img.size();
  strips->updateImage(i, img, chgid>0);

  if (i!=curr)
    return;

  pDebug() << "LightTable::updateImage current slide" << i << img.size();
  slide->updateImage(i, img, chgid>0);
  pDebug() << "LightTable::updateImage current done" << i << img.size();
}

void LightTable::rescan(bool rebuildFilter) {
  if (rebuildFilter)
    populateFilterFromDialog();
  strips->rescan();
}

void LightTable::updateSelectedTiles() {
  if (selection->count() > 10) {
    strips->scene()->update();
  } else {
    QSet<quint64> cc = selection->current();
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
}

void LightTable::openFilterDialog() {
  filterDialog->show();
}

void LightTable::selectAll() {
  selection->selectAll();
  strips->scene()->update();
}

void LightTable::clearSelection() {
  if (curr) {
    simpleSelection(curr, false);
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

void LightTable::applyFilterFromDialog() {
  populateFilterFromDialog();
  quint64 c = db->simpleQuery("select * from current").toULongLong();
  QSqlQuery q = db->query("select * from filter where version==:a", c);
  if (!q.next())
    selectNearestInFilter(c);
  rescan(false);
}

void LightTable::selectNearestInFilter(quint64 /*vsn*/) {
  pDebug() << "Current not in filter - solution NYI";
  // we should select something near the version, but for now:
  select(0);
}

Filter const &LightTable::filter() const {
  return filterDialog->filter();
}

void LightTable::populateFilterFromDialog() {
  Filter f = filterDialog->filter();
  Untransaction t(db);
  db->query("delete from filter");
  db->query("insert into filter select versions.id, photos.id from versions "
           + f.joinClause() + " where " + f.whereClause());
  int N = db->simpleQuery("select count(*) from filter").toInt();
  pDebug() << "Populate Filter" << N;
  db->query("delete from selection"
           " where version not in (select version from filter)");
  if (f.hasCollection())
    emit newCollection(f.collection());
  else
    emit newCollection("");
}

void LightTable::rotateSelected(int dphi) {
  QSet<quint64> vsns = selection->current();

  for (auto id: vsns)
    strips->quickRotate(id, dphi);

  quint64 oldcurr = curr;
  if (vsns.contains(curr)) {
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

  if (curr!=oldcurr)
    makeCurrent(oldcurr);

}

Actions const &LightTable::actions() const {
  return acts;
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
      quint64 v = strips->strip()->versionAbove(current());
      if (v)
        slidePress(v, Qt::LeftButton, 0);
    }}
  << Action{ Qt::Key_Down, "",
      [&]() {
      quint64 v = strips->strip()->versionBelow(current());
      if (v)
        slidePress(v, Qt::LeftButton, 0);
    }}
  << Action{ Qt::Key_Left, "",
      [&]() {
      quint64 v = strips->strip()->versionLeftOf(current());
      if (v)
        slidePress(v, Qt::LeftButton, 0);
    }}
  << Action{ Qt::Key_Right, "",
      [&]() {
      quint64 v = strips->strip()->versionRightOf(current());
      if (v)
        slidePress(v, Qt::LeftButton, 0);
    }}
  << Action { Qt::Key_BracketLeft, "Select previous image",
      [&]() {
      quint64 v = strips->strip()->versionBefore(current());
      if (v)
        slidePress(v, Qt::LeftButton, 0);
    }}
  << Action { Qt::Key_BracketRight, "Select next image",
      [&]() {
      quint64 v = strips->strip()->versionAfter(current());
      if (v)
        slidePress(v, Qt::LeftButton, 0);
    }}
  << Action { Qt::CTRL + Qt::Key_N, "New version from photo",
         [&]() {
      db->newVersion(current(), false);
      rescan();
    }}
  << Action { Qt::CTRL + Qt::Key_D, "New version from current (duplicate)",
         [&]() {
      db->newVersion(current(), true);
      rescan();
    }}
  ;
}

void LightTable::keyPressEvent(QKeyEvent *e) {
  qDebug() << "lighttable:: keypressevent";
  if (acts.activateIf(e)) {
    e->accept();
    return;
  }
}

