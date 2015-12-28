// ColorLabelBar.cpp

#include "ColorLabelBar.h"
#include "PhotoDB.h"
#include "LightTable.h"

ColorLabelBar::ColorLabelBar(PhotoDB *db, LightTable *lighttable,
                             QWidget *parent):
  ActionBar(parent) {
  setWindowTitle("Color label");

  QStringList clrs
    = QString("None Red Yellow Green Blue Purple").split(" ");
  for (int n=0; n<=5; n++) {
    QString lbl;
    if (n)
      lbl = "Label " + clrs[n];
    else
      lbl = "Remove color label";
    acts << Action{Qt::CTRL + Qt::Key_0 + n, lbl,
        [=]() {
        db->query("update versions set colorlabel=:a where id in "
                  " (select version from selection)", n);
        lighttable->updateSelectedTiles();
      }};
    new PAction(acts.last(), QIcon(":icons/color" + clrs[n] + ".svg"), this);
  }

  for (int n=0; n<=5; n++) {
    QString lbl;
    if (n>1)
      lbl = QString("Mark %1 stars").arg(n);
    else if (n==1)
      lbl = "Mark 1 star";
    else
      lbl = "Remove stars";
    acts << Action{Qt::CTRL + Qt::SHIFT + Qt::Key_0 + n, lbl,
        [=]() {
        db->query("update versions set starrating=:a where id in "
                  " (select version from selection)", n);
        lighttable->updateSelectedTiles();
      }};
    parent->addAction(new PAction(acts.last(), this));
  }

  acts << Action{Qt::CTRL + Qt::Key_U, "Mark undecided",
      [=]() {
      db->query("update versions set acceptreject=:a where id in "
                " (select version from selection)",
                int(PhotoDB::AcceptReject::Undecided));
      lighttable->updateSelectedTiles();
    }};
  parent->addAction(new PAction(acts.last(), this));

  acts << Action{Qt::CTRL + Qt::Key_G, "Mark accepted",
      [=]() {
      db->query("update versions set acceptreject=:a where id in "
                " (select version from selection)",
                int(PhotoDB::AcceptReject::Accept));
      lighttable->updateSelectedTiles();
    }};
  parent->addAction(new PAction(acts.last(), this));

  acts << Action{Qt::CTRL + Qt::Key_X, "Mark rejected",
      [=]() {
      db->query("update versions set acceptreject=:a where id in "
                " (select version from selection)",
                int(PhotoDB::AcceptReject::Reject));
      lighttable->updateSelectedTiles();

    }};
  parent->addAction(new PAction(acts.last(), this));

  acts << Action{{Qt::CTRL + Qt::Key_Less, Qt::CTRL + Qt::Key_Comma},
      "Rotate left",
        [=]() { lighttable->rotateSelected(-1); }};
  parent->addAction(new PAction(acts.last(), this));

  acts << Action{{Qt::CTRL + Qt::Key_Greater, Qt::CTRL + Qt::Key_Period},
      "Rotate right",
        [=]() { lighttable->rotateSelected(1); }};
  parent->addAction(new PAction(acts.last(), this));
}

