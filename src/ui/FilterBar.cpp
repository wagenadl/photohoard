// FilterBar.cpp

#include "FilterBar.h"

#include <QMetaType>
#include "PDebug.h"
#include "LightTable.h"

FilterBar::FilterBar(QWidget *parent, LightTable *lt):
  QToolBar(parent), lighttable(lt) {
  setWindowTitle("Filter");

  PQAction *act;

  act = new PQAction{QIcon(":icons/search.svg"),
                     [&]() { lighttable->openFilterDialog(); }};
  actions << Action{Qt::CTRL + Qt::Key_F, "Filter", act};
  addAction(act);

  act = new PQAction{QIcon(":icons/scaleSmaller.svg"),
                     [&]() { qDebug() << "smaller" << lighttable; lighttable->increaseTileSize(1/1.25); }};
  actions << Action{Qt::CTRL + Qt::Key_Minus, "Smaller", act};
  addAction(act);

  act = new PQAction{QIcon(":icons/scaleLarger.svg"),
                     [&]() { lighttable->increaseTileSize(1.25); }};
  actions << Action{Qt::CTRL + Qt::Key_Plus, "Larger", act};
  addAction(act);

  act = new PQAction{[&]() { lighttable->clearSelection(); }};
  actions << Action{Qt::CTRL + Qt::SHIFT + Qt::Key_A, "Clear selection", act};
  parent->addAction(act);

  act = new PQAction{[&]() { lighttable->selectAll(); }};
  actions << Action{Qt::CTRL + Qt::Key_A, "Select all", act};
  parent->addAction(act);
}

FilterBar::~FilterBar() {
}
