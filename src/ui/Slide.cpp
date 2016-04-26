// Slide.cpp

#include "Slide.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QStyleOptionGraphicsItem>
#include "PDebug.h"
#include "StripScene.h"
#include "CMS.h"
#include <QApplication>

bool Slide::mayStartDrag = false;
constexpr int SLIDEMARGINS = 8;

Slide::Slide(quint64 id, Slidestrip *parent):
  QGraphicsItem(parent), parent(parent), id(id) {
  tilesize = 128;
  setPos(1e6, 1e6);
  StripScene *fs = dynamic_cast<StripScene *>(scene());
  if (fs)
    fs->markSlideFor(id, this);
  else
    COMPLAIN("Slide not in a scene - won't show image");
}

Slide::~Slide() {
  StripScene *fs = dynamic_cast<StripScene *>(scene());
  if (fs)
    fs->dropSlideFor(id);
  else
    CRASH("Slide not in a scene");
}

void Slide::updateImage(Image16 const &img1, bool chgd) {
  if (!chgd) {
    if (img1.size().isContainedIn(pm.size())
	|| img1.size().isContainedIn(img.size()))
      return;
  }
  pm = QPixmap();
  if (isVisible()) {
    int ims = tilesize - SLIDEMARGINS;
    if (CMS::monitorTransform.isValid())
      img = CMS::monitorTransform
        .apply(img1.scaledToFitSnuglyIn(PSize(ims, ims)));
    else
      img = img1;
    update();
  } else {
    img = Image16(); // Isn't that right?
  }
}

void Slide::quickRotate(int dphi) {
  dphi = dphi & 3;
  if (dphi==0 || pm.isNull())
    return;
  
  Image16 im = Image16(pm.toImage());
  switch (dphi) {
  case 0:
    break;
  case 1:
    im.rotate90CW();
    break;
  case 2:
    im.rotate180();
    break;
  case 3:
    im.rotate90CCW();
  }
  pm = QPixmap::fromImage(im.toQImage());
  update();
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
    = parent->database()->simpleQuery("select version from currentvsn")
    .toULongLong() == id;
  bool isSelected = isCurrent
    ? true
    : parent->database()->constQuery("select 1 from selection"
                                     " where version==:a limit 1", id).next();

  QSqlQuery q = parent->database()
    ->query("select colorlabel, starrating, acceptreject"
            " from versions where id==:a", id);
  ASSERT(q.next());

  int colorLabel = q.value(0).toInt();
  int starRating = q.value(1).toInt();
  PhotoDB::AcceptReject acceptReject
    = PhotoDB::AcceptReject(q.value(2).toInt());

  painter->setPen(QPen(Qt::NoPen));

  QColor b0 = colorLabelColor(colorLabel);
  QColor b = isCurrent ? QColor("#ffee00")
    : isSelected ? QColor("#ff8800") : b0;
  int dx = 1; //isCurrent ? 2: 1;
  QColor btl = isCurrent ? b.darker()
    : isSelected ? b.darker() : b.lighter();
  QColor bbr = isCurrent ? b.lighter()
    : isSelected ? b.lighter() : b.darker();
  painter->setBrush(bbr);
  painter->drawRoundedRect(r.adjusted(2*dx, 2*dx, 0, 0), 4, 4);
  painter->setBrush(btl);
  painter->drawRoundedRect(r.adjusted(0, 0, -2*dx, -2*dx), 4, 4);
  if (isCurrent || isSelected) {
    painter->setBrush(b);
    painter->drawRoundedRect(r.adjusted(dx, dx, -dx, -dx), 4, 4);
    dx = 3;
  }
  painter->setBrush(b0);
  painter->drawRoundedRect(r.adjusted(dx, dx, -dx, -dx), 4, 4);

  if (starRating>0) {
    // draw a few stars
    QString star = QString::fromUtf8("★");
    QString txt = "";
    for (int n=0; n<starRating; n++)
      txt += star;
    painter->setPen(QColor("#ffee00"));
    painter->drawText(r.adjusted(dx, 0, 0, -dx),
                      Qt::AlignLeft | Qt::AlignBottom,
                      txt);
  }

  if (acceptReject != PhotoDB::AcceptReject::Undecided) {
    static QString acc = QString::fromUtf8("✔"); // ✓✔√");
    static QString rej = QString::fromUtf8("❌"); // ×❌
    static QString newi = QString::fromUtf8("✶");
    QString txt = "";
    QColor clr;
    switch (acceptReject) {
    case PhotoDB::AcceptReject::Accept:
      txt = acc;
      clr = QColor("#00ff00");
      break;
    case PhotoDB::AcceptReject::Reject:
      txt = rej;
      clr = QColor("red");
      break;
    case PhotoDB::AcceptReject::NewImport:
      txt = newi;
      clr = QColor("#ffff00");
    default:
      break;
    }
    QFont f0 = painter->font();
    QFont f = f0;
    f.setPixelSize(16); // hmm.
    f.setWeight(QFont::Bold);
    painter->setPen(clr);
    painter->setFont(f);
    painter->drawText(r.adjusted(0, 0, -3, 0),
                      Qt::AlignRight | Qt::AlignTop,
                      txt);
    painter->setFont(f0);
  }
  
  int ims = tilesize - SLIDEMARGINS;
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
      parent->requestImage(id);
      return;
    }
    if (!img.isNull()) {
      pm = QPixmap::fromImage(img.scaledToFitSnuglyIn(PSize(ims, ims))
			      .toQImage());
    }
    img = Image16();
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
  mayStartDrag = e->button()==Qt::LeftButton;
}

void Slide::mouseMoveEvent(QGraphicsSceneMouseEvent *e) {
  e->accept();
  if (parent && mayStartDrag &&
      (e->pos()-e->buttonDownPos(Qt::LeftButton)).manhattanLength() > 20) {
    parent->startDrag(id);
    mayStartDrag = false;
  }
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

Slidestrip *Slide::parentStrip() const {
  return parent;
}

void Slide::reload() {
  pm = QPixmap();
  img = Image16();
  update();
}
