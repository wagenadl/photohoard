// AllControls.h

#ifndef ALLCONTROLS_H

#define ALLCONTROLS_H

#include <QTabWidget>
#include <QMap>
#include "Adjustments.h"

class AllControls: public QTabWidget {
  /* AllControls - This class contains all the pages of the controls area,
     including the "sliders", "crop", and "layers" pages. */
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
  void maskChanged(quint64 vsn, int lowestaffected);
  void layerSelected(quint64 vsn, int layer); // zero for base
public slots:
  void setVersion(quint64 vsn);
  /* SETVERSION - Sets all sliders and controls
     Does not signal VALUECHANGED.
  */
private slots:
  void changeFromSliders();
  void changeFromCropper(QRect rect, QSize osize);
  void layerIndexChange(int);
  void changeOfTabIndex();
  void maskChangeFromLayers(int layer);
  void valueChangeFromLayers(int layer);
private:
  void storeInDatabase(Adjustments const &adj);
private:
  class ControlSliders *sliders;
  class CropControls *cropper;
  class LayerDialog *layers;
private:
  class PhotoDB *db;
  quint64 vsn;
  Adjustments adjs; // for base layer
};

#endif
