// LayoutBar.cpp

#include "LayoutBar.h"
#include "PDebug.h"
#include "LightTable.h"

LayoutBar::LayoutBar(LightTable *lighttable, QWidget *parent):
  ActionBar(parent) {
  setObjectName("Layout");

  acts << Action{Qt::Key_F1, "Full grid",
      [=]() { lighttable->setLayout(Layout::FullGrid); }};
  new PAction(acts.last(), QIcon(":icons/layoutGrid.svg"), this);

  acts << Action{Qt::SHIFT + Qt::Key_F2, "Horizontal grid plus photo",
      [=]() { lighttable->setLayout(Layout::HGrid); }};
  new PAction(acts.last(), QIcon(":icons/layoutHGrid.svg"), this);

  acts << Action{Qt::Key_F2, "Vertical grid plus photo",
      [=]() { lighttable->setLayout(Layout::VGrid); }};
  new PAction(acts.last(), QIcon(":icons/layoutVGrid.svg"), this);

  acts << Action{Qt::SHIFT + Qt::Key_F3, "Horizontal line plus photo",
      [=]() { lighttable->setLayout(Layout::HLine); }};
  new PAction(acts.last(), QIcon(":icons/layoutHLine.svg"), this);

  acts << Action{Qt::Key_F3, "Vertical line plus photo",
      [=]() { lighttable->setLayout(Layout::VLine); }};
  new PAction(acts.last(), QIcon(":icons/layoutVLine.svg"), this);

  acts << Action{Qt::Key_F4, "Photo only",
      [=]() { lighttable->setLayout(Layout::FullPhoto); }};
  new PAction(acts.last(), QIcon(":icons/layoutFull.svg"), this);
                    
  acts << Action{Qt::Key_F6, "Toggle date/folder view",
      [=]() { lighttable->setLayout(Layout::ToggleOrg); }};
  new PAction(acts.last(), QIcon(":icons/toggleOrg.svg"), this);

  if (parent) {
    acts << Action{Qt::Key_F11, "Toggle full screen",
        [=]() { if (parent->isFullScreen())
          parent->showNormal();
        else
          parent->showFullScreen();
      }};
    parent->addAction(new PAction(acts.last(), this));

    acts << Action { {Qt::SHIFT + int('@'), Qt::SHIFT + Qt::Key_2},
        "Show/hide layer outlines",
        [this, lighttable]() {
            lighttable->showHideLayers();
        }};
    parent->addAction(new PAction(acts.last(), this));    

  acts << Action{Qt::CTRL + Qt::Key_Minus,
      "Reduce tile size", 
      [=]() { lighttable->increaseTileSize(1/1.25); }};
  new PAction{acts.last(), QIcon(":icons/scaleSmaller.svg"), this};
  /* QIcon(":icons/scaleSmaller.svg") */

  acts << Action{{Qt::CTRL + Qt::Key_Plus, Qt::CTRL + Qt::Key_Equal},
      "Increase tile size", 
      [=]() { lighttable->increaseTileSize(1.25); }};
  new PAction{acts.last(), QIcon(":icons/scaleLarger.svg"), this};
  /* QIcon(":icons/scaleLarger.svg") */

  }    
}
