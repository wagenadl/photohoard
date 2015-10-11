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

LightTable::LightTable(PhotoDB *db, LiveAdjuster *adj, QWidget *parent):
  QSplitter(parent), db(db), adjuster(adj) {
  setObjectName("LightTable");
  curr = 0;
  lay=lastlay=LayoutBar::Action::VGrid;
  showmax = false;
  
  bool oldcrash = db->simpleQuery("select count(*) from starting").toInt()>0;
  pDebug() << "Hello world";
  if (oldcrash) {
    Untransaction t(db);
    db->query("update current set version=null");
    db->query("delete from expanded");
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

  connect(strips, SIGNAL(needImage(quint64, QSize)),
	  this, SIGNAL(needImage(quint64, QSize)));
  connect(strips, SIGNAL(pressed(quint64,
				 Qt::MouseButton, Qt::KeyboardModifiers)),
	  this, SLOT(slidePress(quint64,
                                Qt::MouseButton, Qt::KeyboardModifiers)));
  connect(strips, SIGNAL(idealSizeChanged()), SLOT(resizeStrip()));
  connect(strips, SIGNAL(typedColorLabel(ColorLabelBar::Action)),
          SLOT(setColorLabel(ColorLabelBar::Action)));
  connect(strips->scene(),
          SIGNAL(pressed(Qt::MouseButton, Qt::KeyboardModifiers)),
          this, SLOT(bgPress(Qt::MouseButton, Qt::KeyboardModifiers)));

  connect(slide, SIGNAL(typedColorLabel(ColorLabelBar::Action)),
          SLOT(setColorLabel(ColorLabelBar::Action)));
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


void LightTable::setLayout(LayoutBar::Action act) {
  lastlay = lay;
  switch (act) {
  case LayoutBar::Action::FullGrid:
    strips->setArrangement(Strip::Arrangement::Grid);
    strips->show();
    slide->hide();
    lay = act;
    break;
  case LayoutBar::Action::HGrid:
    strips->setArrangement(Strip::Arrangement::Grid);
    setOrientation(Qt::Vertical);
    if (lay==LayoutBar::Action::HLine
        || lay==LayoutBar::Action::VLine
        || lay==LayoutBar::Action::FullPhoto)
      setSizes(QList<int>() << lastgridsize << height()-lastgridsize);
    ensureReasonableGridSize();
    strips->show();
    slide->show();
    lay = act;
    break;
  case LayoutBar::Action::VGrid:
    strips->setArrangement(Strip::Arrangement::Grid);
    setOrientation(Qt::Horizontal);
    if (lay==LayoutBar::Action::HLine
        || lay==LayoutBar::Action::VLine
        || lay==LayoutBar::Action::FullPhoto)
      setSizes(QList<int>() << lastgridsize << width()-lastgridsize);
    ensureReasonableGridSize();
    strips->show();
    slide->show();
    lay = act;
    break;
  case LayoutBar::Action::HLine: {
    if (lay==LayoutBar::Action::HGrid
        || lay==LayoutBar::Action::HGrid)
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
  case LayoutBar::Action::VLine: {
    if (lay==LayoutBar::Action::HGrid
        || lay==LayoutBar::Action::VGrid)
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
  case LayoutBar::Action::FullPhoto:
    if (lay==LayoutBar::Action::HGrid
        || lay==LayoutBar::Action::VGrid)
      lastgridsize = sizes()[0];
    strips->hide();
    slide->show();
    lay = act;
    break;
  case LayoutBar::Action::Line:
    if (orientation()==Qt::Vertical)
      setLayout(LayoutBar::Action::VLine);
    else
      setLayout(LayoutBar::Action::HLine);
    return;
  case LayoutBar::Action::HalfGrid:
    if (orientation()==Qt::Vertical)
      setLayout(LayoutBar::Action::VGrid);
    else
      setLayout(LayoutBar::Action::HGrid);
    return;
  case LayoutBar::Action::ToggleFullPhoto:
    if (lay==LayoutBar::Action::FullPhoto)
      setLayout(lastlay);
    else
      setLayout(LayoutBar::Action::FullPhoto);
    return;
  case LayoutBar::Action::ToggleFullScreen:
    pDebug() << "ToggleFullScreen NYI";
    break;
  case LayoutBar::Action::ToggleOrg:
    strips->toggleOrganization();
    break;
  case LayoutBar::Action::N:
    break;
  }
  if (lastlay==LayoutBar::Action::FullGrid && lay!=lastlay) {
    emit needImage(curr, displaySize());
    requestLargerImage();
  }
  if (lay!=LayoutBar::Action::FullPhoto)
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
    QSqlQuery q = db->query("select photos.width, photos.height, photos.orient"
                           " from photos inner join versions"
                           " on photos.id=versions.photo"
                           " where versions.id==:a", i);
    if (!q.next()) 
      throw NoResult(__FILE__, __LINE__);
    int w = q.value(0).toInt();
    int h =  q.value(1).toInt();
    Exif::Orientation ori = Exif::Orientation(q.value(2).toInt());
    q.finish();
    QSize ns = (ori==Exif::CW || ori==Exif::CCW) ? QSize(h, w): QSize(w, h);
    slide->newImage(ns);
  } else {
    slide->clear();
    adjuster->clear();
  }

  curr = i;
  emit newCurrent(curr);
  emit newZoom(slide->currentZoom());
  if (curr>0 && lay!=LayoutBar::Action::FullGrid) {
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
  strips->updateImage(i, img);

  if (i==curr)
    slide->updateImage(img, true);
}

void LightTable::updateImage(quint64 i, Image16 img) {
  if (i==curr)
    pDebug() << "LightTable::updateImage current" << i << img.size();
  strips->updateImage(i, img);

  if (i!=curr)
    return;

  pDebug() << "LightTable::updateImage current slide" << i << img.size();
  slide->updateImage(img);
  if (i==curr) {
    pDebug() << "LightTable::updateImage current done" << i << img.size();
    Database::disableDebug();
  }
}

void LightTable::rescan(bool rebuildFilter) {
  if (rebuildFilter)
    populateFilterFromDialog();
  strips->rescan();
}

void LightTable::setColorLabel(ColorLabelBar::Action a) {
  int color = int(a);
  db->query("update versions set colorlabel=:a where id in "
           " (select version from selection)", color);
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
  if (lay==LayoutBar::Action::HLine)
    setSizes(QList<int>() << strips->idealSize() << height());
  else if (lay==LayoutBar::Action::VLine) 
    setSizes(QList<int>() << strips->idealSize() << width());
  else
    return;
  //  scrollToCurrent();
}  

void LightTable::filterAction(FilterBar::Action a) {
  switch (a) {
  case FilterBar::Action::ClearSelection:
    clearSelection();
    break;
  case FilterBar::Action::SelectAll:
    selectAll();
    break;
  case FilterBar::Action::OpenFilterDialog:
    filterDialog->show();
    break;
  case FilterBar::Action::Larger:
    strips->setTileSize(10*strips->tileSize()/8);
    resizeStrip();
    break;
  case FilterBar::Action::Smaller:
    strips->setTileSize(8*strips->tileSize()/10);
    resizeStrip();
    break;
  default:
    break;
  }
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
