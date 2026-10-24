// LayoutBar.cpp

#include "LayoutBar.h"
#include "PDebug.h"
#include "LightTable.h"
#include "Style.h"

LayoutBar::LayoutBar(LightTable *lighttable, QWidget *parent):
  ActionBar(parent) {
  setObjectName("Layout");

  acts << Action{Qt::Key_F1, "Full grid",
      [=]() { lighttable->setLayout(Layout::FullGrid); }};
  new PAction(acts.last(), Style::instance()->icon("layoutGrid"), this);

  acts << Action{Qt::SHIFT | Qt::Key_F2, "Horizontal grid plus photo",
      [=]() { lighttable->setLayout(Layout::HGrid); }};
  new PAction(acts.last(), Style::instance()->icon("layoutHGrid"), this);

  acts << Action{Qt::Key_F2, "Vertical grid plus photo",
      [=]() { lighttable->setLayout(Layout::VGrid); }};
  new PAction(acts.last(), Style::instance()->icon("layoutVGrid"), this);

  acts << Action{Qt::SHIFT | Qt::Key_F3, "Horizontal line plus photo",
      [=]() { lighttable->setLayout(Layout::HLine); }};
  new PAction(acts.last(), Style::instance()->icon("layoutHLine"), this);

  acts << Action{Qt::Key_F3, "Vertical line plus photo",
      [=]() { lighttable->setLayout(Layout::VLine); }};
  new PAction(acts.last(), Style::instance()->icon("layoutVLine"), this);

  acts << Action{Qt::Key_F4, "Photo only",
      [=]() { lighttable->setLayout(Layout::FullPhoto); }};
  new PAction(acts.last(), Style::instance()->icon("layoutFull"), this);
                    
  acts << Action{Qt::Key_F6, "Toggle date/folder view",
      [=]() { lighttable->setLayout(Layout::ToggleOrg); }};
  new PAction(acts.last(), Style::instance()->icon("toggleOrg"), this);

  if (parent) {
    acts << Action{Qt::Key_F11, "Toggle full screen",
        [=]() { if (parent->isFullScreen())
          parent->showNormal();
        else
          parent->showFullScreen();
      }};
    parent->addAction(new PAction(acts.last(), this));

    acts << Action { {Qt::SHIFT | Qt::Key_At, Qt::SHIFT | Qt::Key_2},
        "Show/hide layer outlines",
        [this, lighttable]() {
            lighttable->showHideLayers();
        }};
    parent->addAction(new PAction(acts.last(), this));    

  acts << Action{Qt::CTRL | Qt::Key_Minus,
      "Reduce tile size", 
      [=]() { lighttable->increaseTileSize(1/1.25); }};
  new PAction{acts.last(), Style::instance()->icon("scaleSmaller"), this};
  /* Style::instance()->icon("scaleSmaller") */

  acts << Action{{Qt::CTRL | Qt::Key_Plus, Qt::CTRL | Qt::Key_Equal},
      "Increase tile size", 
      [=]() { lighttable->increaseTileSize(1.25); }};
  new PAction{acts.last(), Style::instance()->icon("scaleLarger"), this};
  /* Style::instance()->icon("scaleLarger") */

  }    
}
