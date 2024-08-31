// LayerDialog.h

#ifndef LAYERDIALOG_H

#define LAYERDIALOG_H

#include <QWidget>
#include "Adjustments.h"

class LayerDialog: public QWidget {
  Q_OBJECT;
public:
  LayerDialog(class PhotoDB *db, QWidget *parent=0);
  virtual ~LayerDialog();
  int selectedLayer() const;
  Adjustments const *getAll(quint64 vsn, int layer) const;
public slots:
  void setVersion(quint64);
signals:
  void layerSelected(int);
  /* LAYERSELECTED - Emitted when the user switches layers
     LAYERSELECTED(layer) is emitted when the user selects another 
     layer in the dialog. LAYER=0 is the base layer.
     Not emitted when the layer is reset to zero because of SETVERSION.
  */
  void maskEdited(int layer);
  /* MASKEDITED - Emitted when the mask changes
     MASKEDITED(layer) is emitted when the geometric definition of the layer
     is changed. See also VALUESCHANGED. */
  void valuesChanged(int layer);
  /* VALUESCHANGED - Emitted when the user changes a slider on a layer
  */
  void inSliders(bool);
private slots:
  void addLinearLayer();
  void addShapeLayer();
  void addCloneLayer();
  void addHealLayer();
  void deleteLayer();
  void raiseLayer();
  void lowerLayer();
  void showHideLayer();
  //  void showHideMask();
  void newSelection();
  void respondToClick(int, int);
  void changeFromSliders();
private:
  void storeInDatabase(Adjustments const &adj, int lay);
  void selectLayer(int lay);
  void setEyeIcon(int row, bool onoff);
private:
  class Ui_LayerDialog *ui;
  PhotoDB *db;
  quint64 vsn;
  int lastlay;
  class ControlSliders *sliders;
  QMap<int, Adjustments> adjs;
};

#endif
