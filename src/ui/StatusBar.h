// StatusBar.h

#ifndef STATUSBAR_H

#define STATUSBAR_H

#include <QFrame>

class StatusBar: public QFrame {
  Q_OBJECT;
public:
  StatusBar(class PhotoDB *db, QWidget *parent=0);
  virtual ~StatusBar() { }
  virtual QSize sizeHint() const override;
  virtual QSize minimumSizeHint() const override;
public slots:
  void setZoom(double v);
  void setCollection(QString c);
  void setMessage(QString msg);
  void removeMessage();
protected:
  virtual void paintEvent(QPaintEvent *) override;
private:
  class StatusBarD *d;
};

#endif
