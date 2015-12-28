// LayoutBar.cpp

#include "LayoutBar.h"
#include "PDebug.h"
#include "LightTable.h"

LayoutBar::LayoutBar(LightTable *lighttable, QWidget *parent):
  QToolBar(parent) {
  setWindowTitle("Layout");

  actions << Action{Qt::Key_F1, "Full grid",
      [=]() { lighttable->setLayout(Layout::FullGrid); }};
  new PAction(actions.last(), QIcon(":icons/layoutGrid.svg"), this);

  actions << Action{Qt::SHIFT + Qt::Key_F2, "Horizontal grid plus photo",
      [=]() { lighttable->setLayout(Layout::HGrid); }};
  new PAction(actions.last(), QIcon(":icons/layoutHGrid.svg"), this);

  actions << Action{Qt::Key_F2, "Vertical grid plus photo",
      [=]() { lighttable->setLayout(Layout::VGrid); }};
  new PAction(actions.last(), QIcon(":icons/layoutVGrid.svg"), this);

  actions << Action{Qt::SHIFT + Qt::Key_F3, "Horizontal line plus photo",
      [=]() { lighttable->setLayout(Layout::HLine); }};
  new PAction(actions.last(), QIcon(":icons/layoutHLine.svg"), this);

  actions << Action{Qt::Key_F3, "Vertical line plus photo",
      [=]() { lighttable->setLayout(Layout::VLine); }};
  new PAction(actions.last(), QIcon(":icons/layoutVLine.svg"), this);

  actions << Action{Qt::Key_F4, "Photo only",
      [=]() { lighttable->setLayout(Layout::FullPhoto); }};
  new PAction(actions.last(), QIcon(":icons/layoutFull.svg"), this);

  actions << Action{Qt::Key_F5, "Full screen",
      [=]() { lighttable->setLayout(Layout::ToggleFullScreen); }};
  parent->addAction(new PAction(actions.last(), this));
                    
  actions << Action{Qt::Key_F6, "Toggle date/folder view",
      [=]() { lighttable->setLayout(Layout::ToggleOrg); }};
  new PAction(actions.last(), QIcon(":icons/toggleOrg.svg"), this);
}
