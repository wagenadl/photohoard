// LayerDialog.h

#ifndef LAYERDIALOG_H

#define LAYERDIALOG_H

#include <QWidget>

class LayerDialog: public QWidget {
  Q_OBJECT;
public:
  LayerDialog(QWidget *parent=0);
  virtual ~LayerDialog();
public slots:
  void addGradientLayer();
  void addLayer();
  void deleteLayer();
  void raiseLayer();
  void lowerLayer();
  void showHideLayer();
  void showHideMask();
  void respondToClick(int, int);
  void newSelection();
private:
  class Ui_LayerDialog *ui;
};

#endif
