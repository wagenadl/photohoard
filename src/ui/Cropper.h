// Cropper.h

#ifndef CROPPER_H

#define CROPPER_H

#include <QWidget>

class Cropper: public QWidget {
public:
  Cropper(QWidget *parent=0);
  virtual ~Cropper() { }
private:
  class Ui_Cropper *ui;
};

#endif
