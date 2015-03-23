// VScrollArea.h

#ifndef VSCROLLAREA_H

#define VSCROLLAREA_H

#include <QFrame>
#include <QPointer>

class VScrollArea: public QFrame {
  Q_OBJECT;
public:
  VScrollArea(QWidget *parent);
  virtual ~VScrollArea() { }
  void setChild(QWidget *);
  virtual QSize sizeHint() const override;
  virtual QSize minimumSizeHint() const override;
protected slots:
  void scrollTo(int y);
  void childResized(QSize);
protected:
  void resizeEvent(QResizeEvent *) override;
  bool eventFilter(QObject *obj, QEvent *evt) override;
private:
  QPointer<QWidget> child;
  class QScrollBar *vscroll;
};

#endif
