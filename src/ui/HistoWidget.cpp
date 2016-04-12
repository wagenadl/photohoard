// HistoWidget.cpp

#include "HistoWidget.h"
#include <QList>
#include <QPainter>

HistoWidget::HistoWidget(QWidget *parent): QFrame(parent) {
  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
}

HistoWidget::~HistoWidget() {
}

void HistoWidget::setImage(Image16 const &img) {
  histo.setImage(img);
  update();
}

void HistoWidget::paintEvent(QPaintEvent *) {
  QRect r = contentsRect();
  double maxv = 0;
  int N = histo.numberOfBins();
  for (int k=0; k<3; k++) {
    QVector<double> const &hst = histo.channel(k);
    for (int n=1; n<N-1; n++)
      if (hst[n]>maxv)
        maxv = hst[n];
  }
  if (maxv==0)
    maxv = 1;

  QList<QColor> colors;
  colors << QColor("#0000ff");
  colors << QColor("#00ff00");
  colors << QColor("#ff0000");

  QPainter p(this);
  p.setBrush(QColor("#000000"));
  p.setPen(QPen(Qt::NoPen));
  p.drawRect(r);
  p.setCompositionMode(QPainter::CompositionMode_Lighten);
  for (int k=0; k<3; k++) {
    QPolygon po;
    QVector<double> const &hst = histo.channel(k);
    po << QPoint(r.left(), r.bottom());
    for (int n=0; n<N; n++) 
      po << QPoint(r.left()+r.width()*(n+.5)/N,
                   r.bottom()-r.height()*hst[n]/maxv);
    po << QPoint(r.right(), r.bottom());
    p.setPen(QPen(colors[k]));
    p.setBrush(QBrush(colors[k]));
    p.drawPolygon(po);
  }
}

QSize HistoWidget::sizeHint() const {
  return PSize(200, 160);
}

QSize HistoWidget::minimumSizeHint() const {
  return PSize(50, 40);
}
  
void HistoWidget::setVersion(quint64 v) {
  setEnabled(v>0);
  //  if (!v)
    histo.clear();
  update();
}
