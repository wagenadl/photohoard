// ActionBar.h

#ifndef ACTIONBAR_H

#define ACTIONBAR_H

#include <QToolBar>
#include <QAction>

class ActionBar: public QToolBar {
  Q_OBJECT;
public:
  ActionBar(QWidget *parent=0);
  virtual ~ActionBar();
protected slots:
  virtual void trigger(QAction *a)=0;
  void mtrigger(QObject *a);
protected:
  void addHiddenAction(QAction *a);
private:
  QWidget *parent;
  class QSignalMapper *mapper;
};

#endif
