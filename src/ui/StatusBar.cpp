// StatusBar.cpp

#include "StatusBar.h"
#include <QPainter>
#include "PDebug.h"
#include <QTimer>
#include <math.h>

constexpr int SB_Height = 20;

class StatusBarD: public QObject {
public:
  StatusBarD(StatusBar *parent): QObject(parent) {
    db = 0;
    zoom = 0;
    timer = new QTimer(this);
    timer->setInterval(3000);
    timer->setSingleShot(true);
    connect(timer, SIGNAL(timeout()), parent, SLOT(removeMessage()));
  }
public:
  PhotoDB *db;
  double zoom;
  QString col;
  QString msg;
  QTimer *timer;
  int height;
};

StatusBar::StatusBar(PhotoDB *db, QWidget *parent): QFrame(parent) {
  d = new StatusBarD(this);
  d->db = db;
  d->zoom = 0;
  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
  setContentsMargins(3, 0, 3, 0);
  QFontMetrics m(font());
  d->height = m.boundingRect("(0g)").height();
}

QSize StatusBar::sizeHint() const {
  return QSize(200, d->height);
}

QSize StatusBar::minimumSizeHint() const {
  return QSize(40, d->height);
}

void StatusBar::setZoom(double v) {
  d->zoom = v;
  update();
}

void StatusBar::setCollection(QString c) {
  d->col = c;
  update();
}

void StatusBar::setMessage(QString m) {
  d->msg = m;
  d->timer->start();
  update();
}

void StatusBar::removeMessage() {
  d->msg = "";
  update();
}

void StatusBar::paintEvent(QPaintEvent *) {
  QRect r = contentsRect();
  QPainter p(this);
  p.setBrush(QColor("#eeeeee"));
  p.setPen(QPen(Qt::NoPen));
  p.drawRect(r);
  p.setPen(QPen("#000000"));
  if (d->zoom>0 && d->zoom<1e5)
    p.drawText(r, Qt::AlignVCenter | Qt::AlignRight,
               QString("%1%").arg(round(d->zoom*100)));
  if (!d->msg.isEmpty())
    p.drawText(r, Qt::AlignVCenter | Qt::AlignLeft, d->msg);
  else if (!d->col.isEmpty())
    p.drawText(r, Qt::AlignVCenter | Qt::AlignLeft, d->col);
}
