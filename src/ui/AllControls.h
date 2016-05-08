// AllControls.h

#ifndef ALLCONTROLS_H

#define ALLCONTROLS_H

#include <QTabWidget>
#include <QMap>
#include "Adjustments.h"

class AllControls: public QTabWidget {
  Q_OBJECT;
public:
  AllControls(PhotoDB *db, QWidget *parent=0);
  virtual ~AllControls();
  Adjustments const *getAll(quint64 vsn) const;
  /* GETALL - Get values of all sliders
     VSN must be passed in to ensure we are talking about the same version.
  */
  virtual QSize sizeHint() const override;
  static class Actions const &actions();
signals:
  void valuesChanged(quint64 vsn, int lay, Adjustments adj);
  /* VALUECHANGED - Emitted when the user changes a value.
  */
  void visualizeLayer(int layer); // zero for none
public slots:
  void setVersion(quint64 vsn);
  /* SETVERSION - Sets all sliders and controls
     Does not signal VALUECHANGED.
  */
private slots:
  void changeFromSliders();
  void changeFromCropper(QRect rect, QSize osize);
  void setLayer(int);
  void changeOfIndex();
private:
  void storeInDatabase(Adjustments const &adj);
private:
  class ControlSliders *sliders;
  class CropControls *cropper;
  class LayerDialog *layers;
private:
  class PhotoDB *db;
  quint64 vsn;
  int lay;
  QMap<int, Adjustments> adjs; 
};

#endif
