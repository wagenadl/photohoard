// AllControls.h

#ifndef ALLCONTROLS_H

#define ALLCONTROLS_H

#include <QTabWidget>
#include <QMap>
#include "Adjustments.h"

class AllControls: public QTabWidget {
  Q_OBJECT;
public:
  AllControls(QWidget *parent=0);
  virtual ~AllControls();
  Adjustments const &getAll() const;
  virtual QSize sizeHint() const override;
  static class Actions const &actions();
signals:
  void valueChanged(QString adjustmentname, double value);
  /* VALUECHANGED - Emitted when the user changes the value.
     Note that the key is an adjustment name (see AdjustmentDefs.h) rather
     than a slider name. Mostly these are 1:1, but not quite.
  */
public slots:
  void setAll(Adjustments const &vv, QSize osize);
  /* SETALL - Sets all sliders and controls
     Does not signal VALUECHANGED.
  */
private slots:
  void changeFromSliders(QString adjuster, double value);
  void changeFromCropper(QRect rect, QSize osize);
private:
  class ControlSliders *sliders;
  class CropControls *cropper;
};

#endif
