// FilterBar.cpp

#include "FilterBar.h"

#include <QMetaType>
#include "PDebug.h"
#include "LightTable.h"

FilterBar::FilterBar(LightTable *lighttable, QWidget *parent):
  ActionBar(parent) {
  setObjectName("Filter");

  acts << Action{Qt::CTRL + Qt::Key_F, "Filter",
                    [=]() { lighttable->openFilterDialog(); }};
  new PAction{acts.last(), QIcon(":icons/search.svg"), this};

  acts << Action{Qt::CTRL + Qt::Key_Minus, "Reduce tile size", 
                     [=]() { lighttable->increaseTileSize(1/1.25); }};
  new PAction{acts.last(), QIcon(":icons/scaleSmaller.svg"), this};

  acts << Action{{Qt::CTRL + Qt::Key_Plus, Qt::CTRL + Qt::Key_Equal},
                    "Increase tile size", 
                    [=]() { lighttable->increaseTileSize(1.25); }};
  new PAction{acts.last(), QIcon(":icons/scaleLarger.svg"), this};

  acts << Action{Qt::CTRL + Qt::SHIFT + Qt::Key_A, "Clear selection", 
                    [=]() { lighttable->clearSelection(); }};
  parent->addAction(new PAction{acts.last(), this});

  acts << Action{Qt::CTRL + Qt::Key_A, "Select all", 
                    [=]() { lighttable->selectAll(); }};
  parent->addAction(new PAction{acts.last(), this});
}

