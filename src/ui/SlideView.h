// SlideView.h

#ifndef SLIDEVIEW_H

#define SLIDEVIEW_H

#include <QLabel>
#include "Image16.h"
#include "ColorLabelBar.h"
#include "Action.h"

class SlideView: public QFrame {
  Q_OBJECT;
public:
  SlideView(QWidget *parent=0);
  virtual ~SlideView();
  PSize desiredSize() const;
  double currentZoom() const;
  double fittingZoom() const;
public slots:
  void newImage(QSize natSize);
  void clear();
  void updateImage(Image16 img, bool force=false);
  void setZoom(double zm);
  void changeZoomLevel(QPoint center, double delta, bool roundtodelta=false);
  void scaleToFit();
signals:
  void needLargerImage();
  void doubleClicked();
  void newSize(QSize);
  void newZoom(double);
protected:
  virtual void keyPressEvent(QKeyEvent *) override;
  virtual void mousePressEvent(QMouseEvent *) override;
  virtual void mouseMoveEvent(QMouseEvent *) override;
  virtual void mouseDoubleClickEvent(QMouseEvent *) override;
  virtual void resizeEvent(QResizeEvent *) override;
  virtual void paintEvent(QPaintEvent *) override;
  virtual void wheelEvent(QWheelEvent *) override;
  virtual void enterEvent(QEvent *) override;
private:
  //  QPointF mapWidgetToImage(QPointF) const;
  //  QPointF mapImageToWidget(QPointF) const;
  void updateRelXY(QPoint);
  void makeActions();
private:
  PSize naturalSize;
  Image16 img;
  double zoom;
  bool fit;
  double relx; // 0 for extreme left/top visible, 1 for extreme right/bot
  double rely; // ... not defined when fit
  PSize lastSize;
  QPoint presspoint;
  double pressrelx, pressrely;
  bool dragging;
  Actions actions;
};

#endif
