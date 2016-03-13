// SliderClipboard.cpp

#include "SliderClipboard.h"
#include <QVBoxLayout>
#include "SliderGroups.h"
#include <QCheckBox>
#include <QSignalMapper>
#include <QPushButton>
#include "PhotoDB.h"
#include "AutoCache.h"
#include "Selection.h"
#include "Tristate.h"
#include "PDebug.h"

SliderClipboard::SliderClipboard(PhotoDB *db, AutoCache *ac, QWidget *parent):
  QScrollArea(parent), db(db), ac(ac) {
  //  setWidget(new QWidget());
  valok = false;
  
  QVBoxLayout *vlay = new QVBoxLayout;
  QSignalMapper *gmap = new QSignalMapper(this);
  connect(gmap, SIGNAL(mapped(QString)), SLOT(groupStateChange(QString)));
  QSignalMapper *smap = new QSignalMapper(this);
  connect(smap, SIGNAL(mapped(QString)), SLOT(sliderStateChange(QString)));  
  SliderGroups sg;
  foreach (QString grp, sg.groups()) {
    Tristate *gc = new Tristate;
    connect(gc, SIGNAL(toggled(bool)), gmap, SLOT(map()));
    gmap->setMapping(gc, grp);
    gc->setText(sg.groupLabel(grp));
    groupControl[grp] = gc;
    vlay->addWidget(gc);

    QFrame *gf = new QFrame;
    QVBoxLayout *vl = new QVBoxLayout;
    foreach (QString sli, sg.sliders(grp)) {
      groupContents[grp].insert(sli);
      containingGroup[sli] = grp;
      QCheckBox *sc = new QCheckBox;
      connect(sc, SIGNAL(toggled(bool)), smap, SLOT(map()));
      smap->setMapping(sc, sli);
      sc->setText(sg.sliderLabel(sli));
      jogs[sli] = sc;
      vl->addWidget(sc);
    }
    gf->setLayout(vl);
    groupFrame[grp] = gf;
    vlay->addWidget(gf);
  }
  vlay->addStretch();

  auto *hlay = new QHBoxLayout();
  auto *but = new QPushButton("All");
  connect(but, SIGNAL(clicked()), SLOT(enableAll()));
  hlay->addWidget(but);
  but = new QPushButton("None");
  connect(but, SIGNAL(clicked()), SLOT(disableAll()));
  hlay->addWidget(but);
  hlay->addStretch();
  vlay->addLayout(hlay);

  hlay = new QHBoxLayout();
  hlay->addStretch();
  but = new QPushButton("Copy");
  connect(but, SIGNAL(clicked()), SLOT(copy()));
  hlay->addWidget(but);
  but = new QPushButton("Apply");
  but->setEnabled(false);
  applyButton = but;
  connect(but, SIGNAL(clicked()), SLOT(apply()));
  hlay->addWidget(but);
  vlay->addLayout(hlay);  
  setLayout(vlay);

  enableAll();
  autoResize();  
}

SliderClipboard::~SliderClipboard() {
}

void SliderClipboard::autoResize() {
  resize(sizeHint());
}  

Adjustments SliderClipboard::values() const {
  return val;
}

QSet<QString> SliderClipboard::mask() const {
  QSet<QString> msk;
  for (auto grp: groupControl.keys()) {
    switch (groupControl[grp]->state()) {
    case Qt::Checked:
      for (auto sli: groupContents[grp])
	msk.insert(sli);
      break;
    case Qt::PartiallyChecked:
      for (auto sli: groupContents[grp])
	if (jogs[sli]->isChecked())
	  msk.insert(sli);
      break;
    case Qt::Unchecked:
      break;
    }
  }
  return msk;
}

void SliderClipboard::get(Adjustments *dest) const {
  for (auto sli: mask()) 
    dest->set(sli, val.get(sli));
}

void SliderClipboard::set(class Adjustments const &vv) {
  for (auto sli: mask())
    val.set(sli, vv.get(sli));
}

void SliderClipboard::setAll(class Adjustments const &vv) {
  val = vv;
}

void SliderClipboard::enableAll(bool on) {
  for (auto it=jogs.begin(); it!=jogs.end(); it++) 
    it.value()->setCheckState(on ? Qt::Checked : Qt::Unchecked);
  for (auto gc: groupControl)
    gc->setState(on ? Tristate::On : Tristate::Off);
  autoResize();
}

void SliderClipboard::disableAll(bool off) {
  enableAll(!off);
}

void SliderClipboard::goNext(QString) {
}

void SliderClipboard::goPrevious(QString) {
}


void SliderClipboard::copy() {
  quint64 c = db->current();
  if (!c)
    return;

  auto a = Adjustments::fromDB(c, *db);
  if (valok)
    set(a);
  else
    setAll(a);
  valok = true;

  applyButton->setEnabled(true);
}

void SliderClipboard::apply() {
  if (!valok)
    return;
  
  Selection sel(db);
  QSet<quint64> vv = sel.current();
  Transaction t(db);
  for (auto v: vv) {
    Adjustments a = Adjustments::fromDB(v, *db);
    get(&a);
    a.writeToDB(v, *db);
  }
  t.commit();
  ac->recache(vv);
  for (auto v: vv)
    emit modified(v);
}

void SliderClipboard::groupStateChange(QString grp) {
  pDebug() << "group state change" << grp;
  ASSERT(groupContents.contains(grp));
  switch (groupControl[grp]->state()) {
  case Tristate::On:
    for (QString sli: groupContents[grp]) 
      jogs[sli]->setChecked(true);
    break;
  case Tristate::Off:
    for (QString sli: groupContents[grp]) 
      jogs[sli]->setChecked(false);
    break;
  case Tristate::Undef:
   break;
  }
}

void SliderClipboard::sliderStateChange(QString sli) {
  pDebug() << "slider state change" << sli;
  ASSERT(containingGroup.contains(sli));
  QString grp = containingGroup[sli];
  bool any = false;
  bool all = true;
  for (QString s: groupContents[grp]) {
    if (jogs[s]->isChecked())
      any = true;
    else
      all = false;
  }
  groupControl[grp]->setState(all ? Tristate::On
                              : any ? Tristate::Undef
                              : Tristate::Off);
}
