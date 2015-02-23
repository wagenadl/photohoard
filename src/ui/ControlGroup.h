// ControlGroup.h

#ifndef CONTROLGROUP_H

#define CONTROLGROUP_H

#include <QFrame>

class ControlGroup: public QFrame {
  Q_OBJECT;
public:
  ControlGroup(QString label, QWidget *parent=0);
  virtual ~ControlGroup();
  bool isExpanded() const { return exp_w->isVisible(); } 
  QString label() const { return lbl; }
  void addWidget(QWidget *);
  void addLayout(QLayout *);
public slots:
  void setLabel(QString);
  void expand();
  void collapse();
protected:
  virtual void paintEvent(QPaintEvent *) override;
  virtual void mousePressEvent(QMouseEvent *) override;
  virtual void resizeEvent(QResizeEvent *) override;
  virtual void keyPressEvent(QKeyEvent *) override;
private:
  QString lbl;
  QWidget *exp_w;
  QWidget *col_w;
  class QLabel *hdr;
  class QRadioButton *expander;
  class QVBoxLayout *my_lay, *exp_lay;
  class QHBoxLayout *col_lay;
};

#endif
