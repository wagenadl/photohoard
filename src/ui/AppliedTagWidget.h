// AppliedTagWidget.h

#ifndef APPLIEDTAGWIDGET_H

#define APPLIEDTAGWIDGET_H

#include <QFrame>

class AppliedTagWidget: public QFrame {
  Q_OBJECT;
public:
  AppliedTagWidget(int id, QString name, QWidget *parent=0);
  virtual ~AppliedTagWidget() { }
  virtual QSize sizeHint() const override;
  virtual QSize minimumSizeHint() const override;
public slots:
  void setInclusion(bool inCur, bool inAll);
signals:
  void addClicked(int);
  void removeClicked(int);
protected:
  virtual void enterEvent(QEvent *) override;
  virtual void leaveEvent(QEvent *) override;
  virtual void paintEvent(QPaintEvent *) override;
  virtual void mousePressEvent(QMouseEvent *) override;
private:
  QRect addButtonPlace() const;
  QRect removeButtonPlace() const;
  int buttonSize() const;
private:
  int id;
  QString name;
  bool mouseover;
  bool inall, incur;
};

#endif
