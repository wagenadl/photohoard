// AppliedTagWidget.h

#ifndef APPLIEDTAGWIDGET_H

#define APPLIEDTAGWIDGET_H

#include <QFrame>


/* This is the widget at the bottom-right of the main window where
   current tags are shown. It also offers some editing functionality.

   A tag is shown BLACK if it has been applied to all of the selection
   A tag is shown BLUE if it has been applied to a subset of the selection
   The RED (-) button removes the tag from the entire selection
   The GREEN (+) button spreads it to the rest of the selection
*/

class AppliedTagWidget: public QFrame {
  Q_OBJECT;
public:
  AppliedTagWidget(int id, QString name, bool ro, QWidget *parent=0);
  virtual ~AppliedTagWidget() { }
  virtual QSize sizeHint() const override;
  virtual QSize minimumSizeHint() const override;
public slots:
  void setInclusion(bool inCur, bool inAll);
signals:
  void addClicked(int);
  void removeClicked(int);
protected:
  virtual void enterEvent(QEnterEvent *) override;
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
  bool ro;
};

#endif
