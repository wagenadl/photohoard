// ColorLabelBar.h

#ifndef COLORLABELBAR_H

#define COLORLABELBAR_H

#include "ActionBar.h"

class ColorLabelBar: public ActionBar {
  Q_OBJECT;
public:
  enum class Action {
    SetNone=0,
      SetRed,
      SetYellow,
      SetGreen,
      SetBlue,
      SetPurple,
      N
      };
public:
  ColorLabelBar(QWidget *parent);
  virtual ~ColorLabelBar();
signals:
  void triggered(ColorLabelBar::Action a);
private slots:
  void trigger(QAction *);
private:
  QMap<Action, QAction *> actions;
  QMap<QAction *, Action> revmap;
};

#endif
