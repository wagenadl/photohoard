// Cropper.h

#ifndef CROPPER_H

#define CROPPER_H

#include <QWidget>

class Cropper: public QWidget {
  Q_OBJECT;
public:
  enum class RatioMode {
    Auto,
      Landscape,
      Portrait
      };
public:
  Cropper(QWidget *parent=0);
  virtual ~Cropper() { }
public slots:
  void setRect(QRect croprect);
  void setRect(QRect croprect, QSize origsize);
  void setRatio(int top, int bottom);
  void setRatioMode(RatioMode);
  RatioMode ratioMode() const;
signals:
  void rectangleChanged(QRectF croprect, QSize origsize);
private:
  void reflectRect();
private:
  QRect cropRect;
  QSize origSize;
private:
  class Ui_Cropper *ui;
};

#endif
