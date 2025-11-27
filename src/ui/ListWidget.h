// ListWidget.h

#ifndef LISTWIDGET_H

#define LISTWIDGET_H

#include <QWidget>
#include <QStringList>
#include <QLabel>
#include <QVBoxLayout>

class ListWidget: public QWidget {
  Q_OBJECT;
public:
  ListWidget(QWidget *parent=0);
  virtual ~ListWidget();
  void clear();
  void setUniverse(QStringList options);
  QString text() const;
  void up();
  void down();
private:
  void autosize();
  void highlight();
  void step(int);
private:
  bool firstlow; // lowest numbered is at bottom else at top
  QStringList options;
  int idx = -1; // current index or none
private:
  QVBoxLayout *lay;
  QList<QLabel *> linewidgets;
};

#endif
