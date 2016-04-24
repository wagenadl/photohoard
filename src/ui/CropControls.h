// CropControls.h

#ifndef CROPCONTROLS_H

#define CROPCONTROLS_H

#include <QScrollArea>
#include "CropEnums.h"

class CropControls: public QScrollArea {
  Q_OBJECT;
public:
  CropControls(QWidget *parent=0);
  virtual ~CropControls();
  QSize sizeHint() const;
public slots:
  void setAll(class Adjustments const &adj, QSize osize);
  void setAll(class Adjustments const &adj);
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
  void toggleMode();
  void toggleOrient();
  void clickAspect(QString);
  void optimize();
  void customChanged();
  void customConfirmed();
private:
  void reflectAndEmit();
private:
  class CropControlsUi *ui;
  class CropCalc *calc;
  QSize osize;
};

#endif
