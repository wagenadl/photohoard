// FilterBar.cpp

#include "FilterBar.h"

#include <QMetaType>
#include "PDebug.h"
#include "LightTable.h"

FilterBar::FilterBar(LightTable *lt, QWidget *parent):
  QToolBar(parent), lighttable(lt) {
  setWindowTitle("Filter");

  actions << Action{Qt::CTRL + Qt::Key_F, "Filter",
                    [&]() { lighttable->openFilterDialog(); }};
  new PAction{actions.last(), QIcon(":icons/search.svg"), this};

  actions << Action{Qt::CTRL + Qt::Key_Minus, "Smaller", 
                     [&]() { lighttable->increaseTileSize(1/1.25); }};
  new PAction{actions.last(), QIcon(":icons/scaleSmaller.svg"), this};

  actions << Action{{Qt::CTRL + Qt::Key_Plus, Qt::CTRL + Qt::Key_Equal},
                    "Larger", 
                    [&]() { lighttable->increaseTileSize(1.25); }};
  new PAction{actions.last(), QIcon(":icons/scaleLarger.svg"), this};

  actions << Action{Qt::CTRL + Qt::SHIFT + Qt::Key_A, "Clear selection", 
                    [&]() { lighttable->clearSelection(); }};
  parent->addAction(new PAction{actions.last(), this});

  actions << Action{Qt::CTRL + Qt::Key_A, "Select all", 
                    [&]() { lighttable->selectAll(); }};
  parent->addAction(new PAction{actions.last(), this});
}

FilterBar::~FilterBar() {
}
