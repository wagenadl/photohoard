// OverGroupBox.h

#ifndef OVERGROUPBOX_H

#define OVERGROUPBOX_H

#include <QGroupBox>

class OverGroupBox: public QGroupBox {
  Q_OBJECT;
public:
  OverGroupBox(QWidget *parent=0);
  OverGroupBox(QString title, QWidget *parent=0);
  virtual ~OverGroupBox() {}
  bool isPassThroughEnabled(bool) const { return pt; }
public slots:
  void setPassThroughEnabled(bool=true);
protected:
  friend class OGB_Overlay;
  void overlayPressEvent(QMouseEvent *);
  void overlayReleaseEvent(QMouseEvent *);
  virtual void resizeEvent(QResizeEvent *);
private slots:
  void respondToToggle(bool);
private:
  void createOverlay();
private:
  QWidget *overlay;
  bool pt;
};

#endif
