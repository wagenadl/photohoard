// ClickLabel.h

#ifndef CLICKLABEL_H

#define CLICKLABEL_H

#include <QLabel>

class ClickLabel: public QLabel {
  Q_OBJECT;
public:
  ClickLabel(QWidget *parent=0);
  ClickLabel(QString txt, QWidget *parent=0);
  virtual ~ClickLabel();
signals:
  void clicked();
protected:
  void mousePressEvent(QMouseEvent *);
};

#endif
