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

SliderClipboard::SliderClipboard(PhotoDB *db, AutoCache *ac, QWidget *parent):
  QScrollArea(parent), db(db), ac(ac) {
  valok = false;
  
  QVBoxLayout *vlay = new QVBoxLayout;
  QSignalMapper *map = new QSignalMapper(this);
  connect(map, SIGNAL(mapped(QString)), SLOT(groupStateChange(QString)));
  SliderGroups sg;
  foreach (QString grp, sg.groups()) {
    QCheckBox *gc = new QCheckBox;
    connect(gc, SIGNAL(stateChanged(int)), map, SLOT(map()));
    map->setMapping(gc, grp);
    gc->setText(sg.groupLabel(grp));
    gc->setTristate();
    groupControl[grp] = gc;
    vlay->addWidget(gc);

    QFrame *gf = new QFrame;
    QVBoxLayout *vl = new QVBoxLayout;
    foreach (QString sli, sg.sliders(grp)) {
      groupContents[grp].insert(sli);
      reverseMap[sli] = grp;
      QCheckBox *sc = new QCheckBox;
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
    switch (groupControl[grp]->checkState()) {
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

void SliderClipboard::setMask(QSet<QString> msk) {
  disableAll();
  for (auto s: msk)
    jogs[s]->setChecked(true);
  for (auto it=groupContents.begin(); it!=groupContents.end(); it++) {
    bool any = false;
    bool all = true;
    for (auto s: it.value()) {
      if (jogs[s]->isChecked())
	any = true;
      else
	all = false;
    }
    QString grp = it.key();
    if (all) 
      groupControl[grp]->setCheckState(Qt::Checked);
    else if (any) 
      groupControl[grp]->setCheckState(Qt::PartiallyChecked);
    else 
      groupControl[grp]->setCheckState(Qt::Unchecked);
  }
  autoResize();
}

void SliderClipboard::enableAll(bool on) {
  for (auto it=jogs.begin(); it!=jogs.end(); it++) 
    it.value()->setCheckState(on ? Qt::Checked : Qt::Unchecked);
  for (auto gc: groupControl)
    gc->setCheckState(on ? Qt::Checked : Qt::Unchecked);
  for (auto gf: groupFrame)
    gf->hide();
  autoResize();
}

void SliderClipboard::disableAll(bool off) {
  enableAll(!off);
}

void SliderClipboard::enableGroup(QString name, bool on) {
  Q_ASSERT(groupControl.contains(name));
  groupControl[name]->setCheckState(on ? Qt::Checked : Qt::Unchecked);
  groupFrame[name]->hide();
  autoResize();
}

void SliderClipboard::disableGroup(QString name, bool off) {
  enableGroup(name, !off);
}

void SliderClipboard::enable(QString name, bool on) {
  Q_ASSERT(jogs.contains(name));
  jogs[name]->setChecked(on);

    bool any = false;
    bool all = true;
    QString grp = reverseMap[name];
    for (auto s: groupContents[grp]) {
      if (jogs[s]->isChecked())
	any = true;
      else
	all = false;
    }
    if (all) 
      groupControl[grp]->setCheckState(Qt::Checked);
    else if (any) 
      groupControl[grp]->setCheckState(Qt::PartiallyChecked);
    else 
      groupControl[grp]->setCheckState(Qt::Unchecked);
}

void SliderClipboard::disable(QString name, bool off) {
  enable(name, !off);
}

void SliderClipboard::goNext(QString) {
}

void SliderClipboard::goPrevious(QString) {
}


void SliderClipboard::groupStateChange(QString grp) {
  Q_ASSERT(groupControl.contains(grp));
  switch (groupControl[grp]->checkState()) {
  case Qt::Checked:
  case Qt::Unchecked:
    groupFrame[grp]->hide();
    break;
  case Qt::PartiallyChecked:
    groupFrame[grp]->show();
    break;
  }
  autoResize();
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
