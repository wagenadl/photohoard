// ColorLabelBar.h

#ifndef COLORLABELBAR_H

#define COLORLABELBAR_H

#include "ActionBar.h"

class ColorLabelBar: public ActionBar {
  Q_OBJECT;
public:
  enum class Action {
    SetNoColor=0, // order must match PhotoDB::ColorLabel
      SetRed,
      SetYellow,
      SetGreen,
      SetBlue,
      SetPurple,
      Set0Stars, // order must be 0..5
      Set1Star,
      Set2Stars,
      Set3Stars,
      Set4Stars,
      Set5Stars,
      SetUndecided, // order must match PhotoDB::AcceptReject
      SetAccept,
      SetReject,
      RotateLeft,
      RotateRight,
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
