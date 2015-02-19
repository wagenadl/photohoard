// FilterBar.h

#ifndef FILTERBAR_H

#define FILTERBAR_H

#include "ActionBar.h"

class FilterBar: public ActionBar {
  Q_OBJECT;
public:
  enum class Action {
    OpenFilterDialog,
      ApplyFilter,
      ResetFilter,
      Larger,
      Smaller,
      ClearSelection,
      N
      };
public:
  FilterBar(QWidget *parent);
  virtual ~FilterBar();
signals:
  void triggered(FilterBar::Action a);
private slots:
  void trigger(QAction *);
private:
  QMap<Action, QAction *> actions;
  QMap<QAction *, Action> revmap;
};

#endif
