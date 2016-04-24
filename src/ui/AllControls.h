// AllControls.h

#ifndef ALLCONTROLS_H

#define ALLCONTROLS_H

#include <QTabWidget>
#include <QMap>
#include "Adjustments.h"

class AllControls: public QTabWidget {
  Q_OBJECT;
public:
  AllControls(bool ro, QWidget *parent=0);
  virtual ~AllControls();
  Adjustments const &getAll() const;
  virtual QSize sizeHint() const override;
  static class Actions const &actions();
signals:
  void valuesChanged();
  /* VALUECHANGED - Emitted when the user changes a value.
     Use GETALL to get the new values
  */
public slots:
  void setAll(Adjustments const &vv, QSize osize);
  /* SETALL - Sets all sliders and controls
     Does not signal VALUECHANGED.
  */
private slots:
  void changeFromSliders();
  void changeFromCropper(QRect rect, QSize osize);
private:
  class ControlSliders *sliders;
  class CropControls *cropper;
};

#endif
