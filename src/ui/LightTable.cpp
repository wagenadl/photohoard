// LightTable.cpp

#include "LightTable.h"
#include "NoResult.h"
#include "FilmView.h"
#include "SlideView.h"
#include "Datestrip.h"
#include <QDebug>
#include <QScrollBar>
#include "Selection.h"
#include "Slide.h"
#include "FilmScene.h"

LightTable::LightTable(PhotoDB const &db1, QWidget *parent):
  QSplitter(parent), db(db1) {
  setObjectName("LightTable");

  bool oldcrash = db.simpleQuery("select count(*) from starting").toInt()>0;
  if (oldcrash) {
    db.query("update current set version=null");
    db.query("delete from expanded");
  }
  db.query("insert into starting values(1)");
  
  selection = new Selection(db, this);

  film = new FilmView(db);
  addWidget(film);

  slide = new SlideView();
  addWidget(slide);

  tilesize = 80;
  lastgridsize = 3*tilesize+film->verticalScrollBar()->width()+4;
  setStretchFactor(0, 0);
  setStretchFactor(1, 100);
  setSizes(QList<int>() << lastgridsize << width()-lastgridsize);
  lay=lastlay=LayoutBar::Action::VGrid;
  setLayout(lay);
  showmax = false;

  connect(film, SIGNAL(needImage(quint64, QSize)),
	  this, SIGNAL(needImage(quint64, QSize)));
  connect(film, SIGNAL(pressed(quint64,
                               Qt::MouseButton, Qt::KeyboardModifiers)),
	  this, SLOT(slidePress(quint64,
                                Qt::MouseButton, Qt::KeyboardModifiers)));
  connect(slide, SIGNAL(needLargerImage()),
	  this, SLOT(requestLargerImage()));
  connect(film->scene(),
          SIGNAL(pressed(Qt::MouseButton, Qt::KeyboardModifiers)),
          this, SLOT(bgPress(Qt::MouseButton, Qt::KeyboardModifiers)));

  
  quint64 c = db.simpleQuery("select * from current").toULongLong();
  if (c)
    select(c, Qt::NoModifier);

  db.query("delete from starting");
}

LightTable::~LightTable() {
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
        || lay==LayoutBar::Action::HGrid)
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
        || lay==LayoutBar::Action::HGrid)
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
    break;
  case LayoutBar::Action::HalfGrid:
    if (orientation()==Qt::Vertical)
      setLayout(LayoutBar::Action::VGrid);
    else
      setLayout(LayoutBar::Action::HGrid);
    break;
  case LayoutBar::Action::ToggleFullPhoto:
    if (lay==LayoutBar::Action::FullPhoto)
      setLayout(lastlay);
    else
      setLayout(LayoutBar::Action::FullPhoto);
    break;
  case LayoutBar::Action::ToggleFullScreen:
    break;
  case LayoutBar::Action::N:
    break;
  }
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
  if (i==0)
    return;
  if (m & Qt::ControlModifier) {
    // Control: Toggle whether image i is selected
    if (selection->contains(i))
      selection->remove(i);
    else
      selection->add(i);
  } else if (m & Qt::ShiftModifier) {
    // Shift: Select a range from current id to new i
    // Currently, this always adds, but that needs not be the case
    QDateTime a = db.captureDate(db.photoFromVersion(id));
    QDateTime b = db.captureDate(db.photoFromVersion(i));
    if (a>b)
      selection->addDateRange(b, a);
    else
      selection->addDateRange(a, b);
    film->scene()->update();
  } else {
    // Ignore other modifiers for the moment
    if (i==id)
      return;
    if (!selection->contains(i)) {
      bool localupdate = true; // if we have just a few in selection, we'll
      // repaint just those slides, otherwise the whole view
      if (selection->count()<=10) {
        QSet<quint64> ss = selection->current();
        selection->clear();
        for (auto i: ss) {
          Slide *s = film->root()->slideByVersion(i);
          if (s)
            s->update();
        }
      } else {
        localupdate = false;
        selection->clear();
      }
      selection->add(i);
      if (!localupdate)
        film->scene()->update();
    }
  }

  db.query("update current set version=:a", i);
  updateSlide(id);
  updateSlide(i);
  newImage = true;
  id = i;
  emit newCurrent(id);
  requestLargerImage();
}

void LightTable::updateSlide(quint64 i) {
  Slide *s = film->root()->slideByVersion(i);
  if (s)
    s->update();
}  

void LightTable::requestLargerImage() {
  if (slide->isVisible())
    emit needImage(id, slide->desiredSize());
}  

void LightTable::updateImage(quint64 i, QSize s, QImage img) {
  film->updateImage(i, s, img);

  if (i!=id)
    return;

  if (newImage) {
    QSqlQuery q(*db);
    q.prepare("select photos.width, photos.height"
	      " from photos inner join versions"
	      " on photos.id=versions.photo"
	      " where versions.id==:i"
	      " limit 1");
    q.bindValue(":i", id);
    if (!q.exec())
      throw q;
    if (!q.next())
      throw NoResult();
    
    slide->newImage(QSize(q.value(0).toInt(), q.value(1).toInt()));
    newImage = false;
  }

  slide->updateImage(img);
}

void LightTable::rescan() {
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
      Slide *s = film->root()->slideByVersion(vsn);
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
  default:
    break;
  }
}
        
void LightTable::clearSelection() {
  quint64 c = db.simpleQuery("select * from current").toULongLong();
  if (c) {
    select(c, Qt::NoModifier);
  } else {
    if (selection->count()<=10) {
      QSet<quint64> ss = selection->current();
      selection->clear();
      for (auto i: ss) {
        Slide *s = film->root()->slideByVersion(i);
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
  qDebug() << "bgpress " << b << m;
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
  quint64 c = db.simpleQuery("select * from current").toULongLong();
  if (c)
    film->scrollTo(c);
}