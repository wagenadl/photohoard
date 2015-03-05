// SlideView.h

#ifndef SLIDEVIEW_H

#define SLIDEVIEW_H

#include <QLabel>
#include "Image16.h"

class SlideView: public QFrame {
  Q_OBJECT;
public:
  SlideView(QWidget *parent=0);
  virtual ~SlideView();
  QSize desiredSize() const;
  double currentZoom() const;
  double fittingZoom() const;
public slots:
  void newImage(QSize natSize);
  void updateImage(Image16 img);
  void setZoom(double zm);
  void changeZoomLevel(QPoint center, double delta);
  void scaleToFit();
signals:
  void needLargerImage();
  void doubleClicked();
  void newSize(QSize);
protected:
  virtual void keyPressEvent(QKeyEvent *) override;
  virtual void mousePressEvent(QMouseEvent *) override;
  virtual void mouseMoveEvent(QMouseEvent *) override;
  virtual void mouseDoubleClickEvent(QMouseEvent *) override;
  virtual void resizeEvent(QResizeEvent *) override;
  virtual void paintEvent(QPaintEvent *) override;
  virtual void wheelEvent(QWheelEvent *) override;
private:
  //  QPointF mapWidgetToImage(QPointF) const;
  //  QPointF mapImageToWidget(QPointF) const;
private:
  QSize naturalSize;
  Image16 img;
  double zoom;
  bool fit;
  double relx; // 0 for extreme left/top visible, 1 for extreme right/bot
  double rely; // ... not defined when fit
  QSize lastSize;
};

#endif
