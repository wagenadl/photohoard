// HistoWidget.h

#ifndef HISTOWIDGET_H

#define HISTOWIDGET_H

#include <QFrame>
#include "Histogram.h"

class HistoWidget: public QFrame {
  Q_OBJECT;
public:
  HistoWidget(QWidget *parent=0);
  virtual ~HistoWidget();
  virtual QSize sizeHint() const override;
  virtual QSize minimumSizeHint() const override;
public slots:
  void setImage(Image16 const &);
  void setVersion(quint64);
protected:
  void paintEvent(QPaintEvent *);
private:
  Histogram histo;
};

#endif
