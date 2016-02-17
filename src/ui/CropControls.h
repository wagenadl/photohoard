// CropControls.h

#ifndef CROPCONTROLS_H

#define CROPCONTROLS_H

#include <QFrame>
#include "CropEnums.h"

class CropControls: public QFrame {
  Q_OBJECT;
public:
  CropControls(QWidget *parent=0);
  virtual ~CropControls();
public slots:
  void setAll(class Adjustments const &adj, QSize osize);
  void setValue(QString, double);
signals:
  void rectangleChanged(QRect, QSize);
private slots:
  void slideLeft(double);
  void slideRight(double);
  void slideTop(double);
  void slideBottom(double);
  void slideTL(double);
  void slideTR(double);
  void slideBL(double);
  void slideBR(double);
  void gotoSlider(int);
  void toggleMode();
  void toggleOrient();
  void clickAspect(QString);
private:
  class CropControlsUi *ui;
  class CropCalc *calc;
};

#endif
