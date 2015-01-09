// FilterBar.h

#ifndef FILTERBAR_H

#define FILTERBAR_H

#include <QToolBar>
#include <QAction>

class FilterBar: public QToolBar {
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
  void mtrigger(QObject *);
  void trigger(QAction *);
private:
  QMap<Action, QAction *> actions;
  QMap<QAction *, Action> revmap;
};

#endif
