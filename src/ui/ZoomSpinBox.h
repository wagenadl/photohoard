// ZoomSpinBox.h

#ifndef ZOOMSPINBOX_H

#define ZOOMSPINBOX_H

#include <QSpinBox>

class ZoomSpinBox: public QSpinBox {
  Q_OBJECT;
public:
  ZoomSpinBox(QWidget *parent=0);
  virtual ~ZoomSpinBox() { }
  double zoom() const;
  void setZoom(double z);
  void setMinZoom(double z);
  void setMaxZoom(double z);
  virtual void stepBy(int steps) override;
  virtual void fixup(QString &input) const override;
  virtual QValidator::State validate(QString &input, int &pos) const override;
protected:
  virtual StepEnabled stepEnabled() const override;
  void updateText();
private:
  double minz, maxz;
};

#endif
