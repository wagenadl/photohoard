// ColorLabelBar.cpp

#include "ColorLabelBar.h"
#include "PhotoDB.h"
#include "LightTable.h"
#include "PDebug.h"

ColorLabelBar::ColorLabelBar(PhotoDB *db, LightTable *lighttable,
                             QWidget *parent):
  ActionBar(parent) {
  setObjectName("Color label");

  QStringList clrs
    = QString("None Red Yellow Green Blue Purple").split(" ");
  for (int n=0; n<=5; n++) {
    QString lbl;
    if (n)
      lbl = "Label " + clrs[n];
    else
      lbl = "Remove color label";
    acts << Action{Qt::CTRL | Qt::Key_0 + n, lbl,
        [=]() {
          { DBWriteLock lock(db);
            db->query("update versions set colorlabel=:a where id in "
                      " (select version from selection)", n);
          }
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
    acts << Action{Qt::ALT + Qt::Key_0 + n, lbl,
        [=]() {
          { DBWriteLock lock(db);
            db->query("update versions set starrating=:a where id in "
                      " (select version from selection)", n);
          }
          lighttable->updateSelectedTiles();
        }};
    parent->addAction(new PAction(acts.last(), this));
  }

  auto foo = [db](int n) {
               DBWriteLock lock(db);
               db->query("update versions set acceptreject=:a where id in "
                         " (select version from selection)", n);
  };
  acts << Action{Qt::CTRL | Qt::Key_U, "Mark undecided",
      [=]() {
      foo(int(PhotoDB::AcceptReject::Undecided));;
      lighttable->updateSelectedTiles();
    }};
  parent->addAction(new PAction(acts.last(), this));

  acts << Action{Qt::CTRL | Qt::Key_G, "Mark accepted",
      [=]() {
      foo(int(PhotoDB::AcceptReject::Accept));;
      lighttable->updateSelectedTiles();
    }};
  parent->addAction(new PAction(acts.last(), this));

  acts << Action{Qt::CTRL | Qt::Key_X, "Mark rejected",
      [=]() {
      foo(int(PhotoDB::AcceptReject::Reject));
      lighttable->updateSelectedTiles();
    }};
  parent->addAction(new PAction(acts.last(), this));
  
  acts << Action{Qt::CTRL | Qt::Key_J, "Mark new import",
      [=]() {
      foo(int(PhotoDB::AcceptReject::NewImport));
      lighttable->updateSelectedTiles();

    }};
  parent->addAction(new PAction(acts.last(), this));

  acts << Action{{Qt::CTRL | Qt::Key_Less, Qt::CTRL | Qt::Key_Comma},
      "Rotate left",
        [=]() { lighttable->rotateSelected(-1); }};
  parent->addAction(new PAction(acts.last(), this));

  acts << Action{{Qt::CTRL | Qt::Key_Greater, Qt::CTRL | Qt::Key_Period},
      "Rotate right",
        [=]() { lighttable->rotateSelected(1); }};
  parent->addAction(new PAction(acts.last(), this));
}

