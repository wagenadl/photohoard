// CropControls.h

#ifndef CROPCONTROLS_H

#define CROPCONTROLS_H

#include <QWidget>
#include <QRectF>
#include <QVector>

class CropControls: public QWidget {
  Q_OBJECT;
public:
  CropControls(QWidget *parent=0);
public slots:
  void setAll(class Adjustments const &adj, QSize osize);
signals:
  void rectangleChanged(QRect, QSize);
private:
  enum class Mode { Free=0, Aspect=1, Size=2 };
  enum class Orientation { Auto=0, Landscape=1, Portrait=2 };
private slots:
  void changeN(double);
  void changeE(double);
  void changeS(double);
  void changeW(double);
  void changeNE(double);
  void changeSE(double);
  void changeSW(double);
  void changeNW(double);
  void gotoSlider(int);
  void setMode(Mode);
  void setOrientation(Orientation);
  void setAspect(double);
  void setAspectIndex(int);
private:
  void populate();
  void connectAll();
  void addAspects();
  void addAspect(int, int);
  void addAspect(QString, double);
  void addButtons(QVector<class QAbstractButton *> &, QStringList);
  void addSliders();
private:
  QVector<class QAbstractButton *> modeControls;
  QVector<class QAbstractButton *> orientControls;
  QVector<class QAbstractButton *> aspectControls;
  QVector<class GentleJog *> sliders;
  QLineEdit *customAspect;
  QLineEdit *customSize;
  QVector<double> aspectValues;
  class QGridLayout *aspectLayout;
private:
  QRect rect;
  QSize osize;
  QRectF rectf;
};

#endif
