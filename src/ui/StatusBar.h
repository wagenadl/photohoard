// StatusBar.h

#ifndef STATUSBAR_H

#define STATUSBAR_H

#include <QFrame>

class StatusBar: public QFrame {
  Q_OBJECT;
public:
  StatusBar(QWidget *parent=0);
  virtual ~StatusBar() { }
  virtual QSize sizeHint() const override;
  virtual QSize minimumSizeHint() const override;
public slots:
  void setZoom(double v);
  void setCollection(QString c);
protected:
  virtual void paintEvent(QPaintEvent *) override;
private:
  double zoom;
  QString col;
};

#endif
