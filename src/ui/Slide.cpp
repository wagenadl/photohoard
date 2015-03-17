// Slide.cpp

#include "Slide.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QStyleOptionGraphicsItem>
#include "PDebug.h"
#include "FilmScene.h"
#include "CMS.h"

Slide::Slide(quint64 id, Slidestrip *parent):
  QGraphicsItem(parent), parent(parent), id(id) {
  tilesize = 128;
  setPos(1e6, 1e6);
  FilmScene *fs = dynamic_cast<FilmScene *>(scene());
  if (fs)
    fs->markSlideFor(id, this);
  else
    pDebug() << "Slide not in a scene - won't show image";
  dbgstarted = false;
}

Slide::~Slide() {
  FilmScene *fs = dynamic_cast<FilmScene *>(scene());
  if (fs)
    fs->dropSlideFor(id);
  else
    pDebug() << "Slide not in a scene - disaster imminent";
}

void Slide::updateImage(Image16 const &img1) {
  pm = QPixmap();
  if (isVisible()) {
    if (CMS::monitorTransform.isValid())
      img = CMS::monitorTransform.apply(img1);
    else
      img = img1;
    update();
  }
}

void Slide::setTileSize(int pix) {
  tilesize = pix;
  update();
}

void Slide::paint(QPainter *painter,
		  const QStyleOptionGraphicsItem *,
		  QWidget *) {
  QRectF r = boundingRect();
  bool isCurrent
    = parent->database().simpleQuery("select version from current")
    .toULongLong() == id;
  bool isSelected = isCurrent
    ? true
    : parent->database().simpleQuery("select count(*) from selection"
                                     " where version==:a", id).toInt()>0;

  int colorLabel
    = parent->database().simpleQuery("select colorlabel from versions"
                                     " where id==:a", id).toInt();
  painter->setPen(QPen(Qt::NoPen));

  QColor b = colorLabelColor(colorLabel);
  if (isSelected)
    b = b.darker(130);
  if (isCurrent)
    b = b.darker(130);

  int dx = isCurrent ? 2: 1;
  painter->setBrush(isSelected ? b.lighter() : b.darker());
  painter->drawRoundedRect(r.adjusted(2*dx, 2*dx, 0, 0), 4, 4);
  painter->setBrush(isSelected ? b.darker() : b.lighter());
  painter->drawRoundedRect(r.adjusted(0, 0, -2*dx, -2*dx), 4, 4);
  painter->setBrush(b);
  painter->drawRoundedRect(r.adjusted(dx, dx, -dx, -dx), 4, 4);
  int ims = tilesize - 8;
  if (!(pm.width()==ims || pm.height()==ims)) {
    if (img.isNull()) {
      painter->setPen(QPen(QColor(255, 255, 255)));
      painter->setBrush(QBrush(QColor(0, 0, 0)));
      if (pm.isNull()) {
	painter->drawText(r, Qt::AlignCenter, QString("%1").arg(id));
      } else {
	double hrat = double(ims) / pm.width();
	double vrat = double(ims) / pm.height();
	double rat = hrat<vrat ? hrat: vrat;
	PSize tgt(rat*pm.width(), rat*pm.height());
	QRect dst(QPoint(tilesize/2-tgt.width()/2,
			   tilesize/2-tgt.height()/2), tgt);
	painter->drawPixmap(dst, pm);
      }
      if (!dbgstarted) {
	// dbgtime.start();
        // dbgstarted = true;
      }
      parent->requestImage(id);
      return;
    }
    if (dbgstarted) {
      int dt = dbgtime.elapsed();
      dbgstarted = false;
      QTime t = QTime::currentTime();
      pDebug() << "Time" << id << dt
	       << t.msec() + 1000*t.second() + 60*1000*t.minute() + 60*60*1000*t.hour();
    }
    if (!img.isNull())
      pm = QPixmap::fromImage(img.scaledToFitIn(PSize(ims, ims)).toQImage());
    img = Image16(); // no need to keep it ad inf
  }
  painter->drawPixmap(tilesize/2 - pm.width()/2,
		      tilesize/2 - pm.height()/2,
		      pm);
}

void Slide::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e) {
  if (parent)
    parent->slideDoubleClicked(id, e->button(), e->modifiers());
}

void Slide::mousePressEvent(QGraphicsSceneMouseEvent *e) {
  if (parent)
    parent->slidePressed(id, e->button(), e->modifiers());
  e->accept();
}

void Slide::mouseReleaseEvent(QGraphicsSceneMouseEvent *e) {
  if ((e->screenPos() - e->buttonDownScreenPos(e->button()))
      .manhattanLength() < 5)
    if (parent)
      parent->slideClicked(id, e->button(), e->modifiers());
}

 QColor Slide::colorLabelColor(int c) {
   static QMap<int, QColor> cc;
   static QColor bg("#808080");
   if (cc.isEmpty()) {
     cc[1] = QColor("#bb4455");
     cc[2] = QColor("#bbaa44");
     cc[3] = QColor("#66aa66");
     cc[4] = QColor("#3355bb");
     cc[5] = QColor("#aa66aa");
   }
   if (c && cc.contains(c))
     return cc[c];
   else
     return bg;
 }
