// LayerDialog.h

#ifndef LAYERDIALOG_H

#define LAYERDIALOG_H

#include <QWidget>

class LayerDialog: public QWidget {
  Q_OBJECT;
public:
  LayerDialog(class PhotoDB *db, QWidget *parent=0);
  virtual ~LayerDialog();
  int selectedLayer() const;
public slots:
  void setVersion(quint64);
signals:
  void layerSelected(int);
  /* LAYERSELECTED - Emitted when the user switches layers
     LAYERSELECTED(layer) is emitted when the user selects another 
     layer in the dialog. LAYER=0 is the base layer.
     Not emitted when the layer is reset to zero because of SETVERSION.
  */
  void edited();
private slots:
  void addGradientLayer();
  void addLayer();
  void deleteLayer();
  void raiseLayer();
  void lowerLayer();
  void showHideLayer();
  void showHideMask();
  void newSelection();
  void respondToClick(int, int);
private:
  class Ui_LayerDialog *ui;
  PhotoDB *db;
  quint64 vsn;
  int lastlay;
};

#endif
