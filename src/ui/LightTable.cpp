// LightTable.cpp

#include "LightTable.h"
#include "NoResult.h"
#include "FilmView.h"
#include "SlideView.h"
#include "Datestrip.h"
#include "PDebug.h"
#include <QScrollBar>
#include "Selection.h"
#include "Slide.h"
#include "FilmScene.h"
#include "Exif.h"
#include "FilterDialog.h"

LightTable::LightTable(PhotoDB const &db1, LiveAdjuster *adj, QWidget *parent):
  QSplitter(parent), db(db1), adjuster(adj) {
  setObjectName("LightTable");
  id = 0;
  tilesize = 96;
  lastgridsize = 3*tilesize + 4;
  lay=lastlay=LayoutBar::Action::VGrid;
  showmax = false;
  
  bool oldcrash = db.simpleQuery("select count(*) from starting").toInt()>0;
  if (oldcrash) {
    db.query("update current set version=null");
    db.query("delete from expanded");
  }
  db.query("insert into starting values(1)");

  filterDialog = new FilterDialog(db);
  populateFilterFromDialog();
  
  selection = new Selection(db, this);

  film = new FilmView(db);
  addWidget(film);

  slide = new SlideView();
  addWidget(slide);

  lastgridsize = 3*tilesize+film->verticalScrollBar()->width()+4;
  setStretchFactor(0, 0);
  setStretchFactor(1, 100);
  setSizes(QList<int>() << lastgridsize << width()-lastgridsize);
  setLayout(lay);

  connect(film, SIGNAL(needImage(quint64, QSize)),
	  this, SIGNAL(needImage(quint64, QSize)));
  connect(film, SIGNAL(pressed(quint64,
                               Qt::MouseButton, Qt::KeyboardModifiers)),
	  this, SLOT(slidePress(quint64,
                                Qt::MouseButton, Qt::KeyboardModifiers)));

  connect(film->scene(),
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
  
  quint64 c = db.simpleQuery("select * from current").toULongLong();
  if (c)
    select(c);

  db.query("delete from starting");
}

LightTable::~LightTable() {
  delete filterDialog;
}

void LightTable::ensureReasonableGridSize() {
  int s0 = sizes()[0];
  int s1 = 2*tilesize + 5*Strip::labelHeight(tilesize)
    + film->verticalScrollBar()->width() + 4;
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
    film->show();
    film->setArrangement(Strip::Arrangement::Grid);
    film->setTileSize(tilesize);
    slide->hide();
    lay = act;
    break;
  case LayoutBar::Action::HGrid:
    setOrientation(Qt::Vertical);
    if (lay==LayoutBar::Action::HLine
        || lay==LayoutBar::Action::VLine
        || lay==LayoutBar::Action::FullPhoto)
      setSizes(QList<int>() << lastgridsize << height()-lastgridsize);
    ensureReasonableGridSize();
    film->show();
    film->setArrangement(Strip::Arrangement::Grid);
    film->setTileSize(tilesize);
    slide->show();
    lay = act;
    break;
  case LayoutBar::Action::VGrid:
    setOrientation(Qt::Horizontal);
    if (lay==LayoutBar::Action::HLine
        || lay==LayoutBar::Action::VLine
        || lay==LayoutBar::Action::FullPhoto)
      setSizes(QList<int>() << lastgridsize << width()-lastgridsize);
    ensureReasonableGridSize();
    film->show();
    film->setArrangement(Strip::Arrangement::Grid);
    film->setTileSize(tilesize);
    slide->show();
    lay = act;
    break;
  case LayoutBar::Action::HLine:
    if (lay==LayoutBar::Action::HGrid
        || lay==LayoutBar::Action::HGrid)
      lastgridsize = sizes()[0];
    setSizes(QList<int>() << tilesize + film->horizontalScrollBar()->height()
             << height());
    setOrientation(Qt::Vertical);
    film->show();
    film->setArrangement(Strip::Arrangement::Horizontal);
    slide->show();
    lay = act;
    break;
  case LayoutBar::Action::VLine:
    if (lay==LayoutBar::Action::HGrid
        || lay==LayoutBar::Action::VGrid)
      lastgridsize = sizes()[0];
    setSizes(QList<int>() << tilesize + film->verticalScrollBar()->width()
             << width());
    setOrientation(Qt::Horizontal);
    film->show();
    film->setArrangement(Strip::Arrangement::Vertical);
    slide->show();
    lay = act;
    break;
  case LayoutBar::Action::FullPhoto:
    if (lay==LayoutBar::Action::HGrid
        || lay==LayoutBar::Action::VGrid)
      lastgridsize = sizes()[0];
    film->hide();
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
    film->toggleOrganization();
    break;
  case LayoutBar::Action::N:
    break;
  }
  if (lastlay==LayoutBar::Action::FullGrid && lay!=lastlay) {
    emit needImage(id, displaySize());
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

void LightTable::select(quint64 i, Qt::KeyboardModifiers m) {
  if (m & Qt::ControlModifier) {
    // Control: Toggle whether image i is selected
    if (selection->contains(i))
      selection->remove(i);
    else
      selection->add(i);
    emit newSelection();
  } else if (m & Qt::ShiftModifier) {
    // Shift: Select a range from current id to new i
    // Currently, this always adds, but that needs not be the case
    QDateTime a = db.captureDate(db.photoFromVersion(id));
    QDateTime b = db.captureDate(db.photoFromVersion(i));
    if (a>b)
      selection->addDateRange(b, a);
    else
      selection->addDateRange(a, b);
    emit newSelection();
    film->scene()->update();
  } else {
    // Ignore other modifiers for the moment
    if (i==id)
      return;
    if (i>0 && !selection->contains(i)) {
      bool localupdate = true; // if we have just a few in selection, we'll
      // repaint just those slides, otherwise the whole view
      if (selection->count()<=10) {
        QSet<quint64> ss = selection->current();
        selection->clear();
        for (auto i: ss) {
          Slide *s = film->strip()->slideByVersion(i);
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
        film->scene()->update();
    }
  }

  db.query("update current set version=:a", i);
  updateSlide(id);
  updateSlide(i);

  if (i>0) {
    QSqlQuery q = db.query("select photos.width, photos.height, photos.orient"
                           " from photos inner join versions"
                           " on photos.id=versions.photo"
                           " where versions.id==:a", i);
    if (!q.next()) 
      throw NoResult(__FILE__, __LINE__);
    int w = q.value(0).toInt();
    int h =  q.value(1).toInt();
    Exif::Orientation ori = Exif::Orientation(q.value(2).toInt());
    QSize ns = (ori==Exif::CW || ori==Exif::CCW) ? QSize(h, w): QSize(w, h);
    slide->newImage(ns);
  } else {
    slide->clear();
    adjuster->clear();
  }

  id = i;
  emit newCurrent(id);
  if (id>0 && lay!=LayoutBar::Action::FullGrid) {
    emit needImage(id, displaySize());
    requestLargerImage();
  }

  film->scrollIfNeeded();
}    

void LightTable::updateSlide(quint64 i) {
  Slide *s = film->strip()->slideByVersion(i);
  if (s)
    s->update();
}

PSize LightTable::displaySize() const {
  if (slide->isVisible())
    return slide->desiredSize();
  else
    return PSize::square(film->strip()->tileSize());
}

void LightTable::requestLargerImage() {
  if (slide->isVisible()) {
    // also request from cache if helpful and readily available!
    adjuster->requestAdjusted(id, slide->desiredSize());
  } else {
    adjuster->markVersionAndSize(id, slide->desiredSize());
  }
}

void LightTable::updateAdjusted(Image16 img, quint64 i) {
  if (img.isNull())
    return;
  film->updateImage(i, img);

  if (i==id)
    slide->updateImage(img, true);
}

void LightTable::updateImage(quint64 i, Image16 img) {
  film->updateImage(i, img);

  if (i!=id)
    return;

  slide->updateImage(img);
}

void LightTable::rescan(bool rebuildFilter) {
  if (rebuildFilter)
    populateFilterFromDialog();
  film->rescan();
}

void LightTable::setColorLabel(ColorLabelBar::Action a) {
  int color = int(a);
  db.query("update versions set colorlabel=:a where id in "
           " (select version from selection)", color);
  if (selection->count() > 10) {
    film->scene()->update();
  } else {
    QSet<quint64> cc = selection->current();
    for (auto vsn: cc) {
      Slide *s = film->strip()->slideByVersion(vsn);
      if (s)
        s->update();
    }
  }
}

void LightTable::filterAction(FilterBar::Action a) {
  switch (a) {
  case FilterBar::Action::Smaller: case FilterBar::Action::Larger:
    if (a==FilterBar::Action::Smaller) {
      tilesize = tilesize * 8/10;
      if (tilesize<50)
        tilesize = 50;
    } else {
      tilesize = tilesize * 10/8;
      if (tilesize>1024)
        tilesize = 1024;
    }
    film->setTileSize(tilesize);
    if (lay==LayoutBar::Action::HLine)
      setSizes(QList<int>() << tilesize + film->horizontalScrollBar()->height()
               << height());
    else if (lay==LayoutBar::Action::VLine) 
      setSizes(QList<int>() << tilesize + film->verticalScrollBar()->width()
               << width());
    scrollToCurrent();
    break;
  case FilterBar::Action::ClearSelection:
    clearSelection();
    break;
  case FilterBar::Action::OpenFilterDialog:
    filterDialog->show();
  default:
    break;
  }
}
        
void LightTable::clearSelection() {
  quint64 c = db.simpleQuery("select * from current").toULongLong();
  if (c) {
    select(c);
  } else {
    if (selection->count()<=10) {
      QSet<quint64> ss = selection->current();
      selection->clear();
      for (auto i: ss) {
        Slide *s = film->strip()->slideByVersion(i);
        if (s)
          s->update();
      }
    } else {
      selection->clear();
      film->scene()->update();
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
  film->scrollToCurrent();
}

void LightTable::applyFilterFromDialog() {
  populateFilterFromDialog();
  quint64 c = db.simpleQuery("select * from current").toULongLong();
  QSqlQuery q = db.query("select * from M.filter where version==:a", c);
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
  db.query("delete from M.filter");
  db.query("insert into M.filter select versions.id, photos.id from versions "
           + f.joinClause() + " where " + f.whereClause(db));
  int N = db.simpleQuery("select count(*) from M.filter").toInt();
  pDebug() << "Populate Filter" << N;

}
