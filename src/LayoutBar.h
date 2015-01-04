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
      Line, // not a layout: selects HLine or VLine
      HalfGrid, // not a layout: selects HGrid or VGrid
      ToggleFullPhoto, // not a layout: selects FullPhoto or previous layout
      ToggleFullScreen, // not a layout: toggles full screen display
      N
      };
public:
  LayoutBar(QWidget *parent);
  virtual ~LayoutBar();
signals:
  void triggered(LayoutBar::Action a);
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
