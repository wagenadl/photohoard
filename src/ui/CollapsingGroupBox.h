// CollapsingGroupBox.h

#ifndef COLLAPSINGGROUPBOX_H

#define COLLAPSINGGROUPBOX_H

#include <QGroupBox>
#include <QMap>

class CollapsingGroupBox: public QGroupBox {
  Q_OBJECT;
public:
  CollapsingGroupBox(QString const &title, QWidget *parent=0);
  CollapsingGroupBox(QWidget *parent=0);
  virtual ~CollapsingGroupBox();
  void setTitle(QString);
  QString title() const;
  void open();
  void collapse();
  void mouseDoubleClickEvent(QMouseEvent *) override;
private:
  QString openTitle() const;
  QString closedTitle() const;
private:
  QString _title;
  bool _open;
  QMap<QWidget *, bool> wvis;
  QMargins mrg;
  int sp;
};

#endif
