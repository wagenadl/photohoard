// LayoutBar.h

#ifndef LAYOUTBAR_H

#define LAYOUTBAR_H

#include <QToolBar>
#include <QAction>

class LayoutBar: public QToolBar {
  Q_OBJECT;
public:
  enum class Action {
    FullGrid=0,
      HGrid,
      VGrid,
      HLine,
      VLine,
      FullPhoto,
      Line,
      HalfGrid,
      ToggleFullScreen,
      ToggleFullPhoto,
      N
      };
public:
  LayoutBar(QWidget *parent);
  virtual ~LayoutBar();
signals:
  void triggered(Action a);
private slots:
  void trigger(QAction *);
private:
  Action currentLayout;
  Action previousLayout;
private:
  QMap<Action, QAction *> actions;
  QMap<QAction *, Action> revmap;
};

#endif
