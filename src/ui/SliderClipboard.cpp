// SliderClipboard.cpp

#include "SliderClipboard.h"
#include <QVBoxLayout>
#include "SliderGroups.h"
#include <QCheckBox>
#include <QSignalMapper>
#include <QPushButton>
#include "SessionDB.h"
#include "AutoCache.h"
#include "Selection.h"
#include "Tristate.h"
#include "PDebug.h"
#include <QScrollArea>
#include <QGridLayout>
#include <QLabel>
#include "ControlSliders.h"
#include <math.h>

SliderClipboard::SliderClipboard(SessionDB *db, AutoCache *ac, QWidget *parent):
  QDialog(parent), db(db), ac(ac) {
  sa = new QScrollArea();
  sa->setWidget(new QWidget());
  sa->setWidgetResizable(true);
  valok = false;
  
  QVBoxLayout *vlay = new QVBoxLayout;
  vlay->setSpacing(2);
  vlay->setContentsMargins(9, 9, 9, 4);
  QSignalMapper *gmap = new QSignalMapper(this);
  connect(gmap, SIGNAL(mapped(QString)), SLOT(groupStateChange(QString)));
  QSignalMapper *smap = new QSignalMapper(this);
  connect(smap, SIGNAL(mapped(QString)), SLOT(sliderStateChange(QString)));  
  SliderGroups const &sg(SliderGroups::sliderGroups());
  foreach (QString grp, sg.groups()) {
    Tristate *gc = new Tristate;
    connect(gc, SIGNAL(toggled(bool)), gmap, SLOT(map()));
    gmap->setMapping(gc, grp);
    gc->setText(sg.groupLabel(grp));
    groupControl[grp] = gc;
    vlay->addWidget(gc);

    auto *gf = new QFrame;
    auto *vl = new QGridLayout;
    vl->setSpacing(0);
    vl->setContentsMargins(9, 0, 0, 3);
    int row = 0;
    foreach (QString sli, sg.sliders(grp)) {
      groupContents[grp].insert(sli);
      containingGroup[sli] = grp;
      QCheckBox *sc = new QCheckBox;
      connect(sc, SIGNAL(toggled(bool)), smap, SLOT(map()));
      smap->setMapping(sc, sli);
      sc->setText(sg.sliderLabel(sli));
      jogs[sli] = sc;
      vl->addWidget(sc, row, 0);
      QLabel *lbl = new QLabel;
      lbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      labels[sli] = lbl;
      vl->addWidget(lbl, row, 1);
      ++row;
    }
    gf->setLayout(vl);
    groupFrame[grp] = gf;
    vlay->addWidget(gf);
  }
  vlay->addStretch(1);
  sa->widget()->setLayout(vlay);
  sa->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  vlay = new QVBoxLayout;
  vlay->setContentsMargins(0,0,0,0);
  vlay->setSpacing(2);
  vlay->addWidget(sa);

  auto *hlay = new QHBoxLayout();
  hlay->setContentsMargins(6, 4, 26, 2);
  auto *but = new QPushButton("All");
  connect(but, SIGNAL(clicked()), SLOT(enableAll()));
  hlay->addWidget(but);
  but = new QPushButton("None");
  connect(but, SIGNAL(clicked()), SLOT(disableAll()));
  hlay->addWidget(but);
  hlay->addStretch();
  vlay->addLayout(hlay);

  hlay = new QHBoxLayout();
  hlay->setContentsMargins(26, 2, 6, 9);
  hlay->addStretch();
  but = new QPushButton("Copy");
  connect(but, SIGNAL(clicked()), SLOT(copy()));
  hlay->addWidget(but);
  but = new QPushButton("Apply");
  applyButton = but;
  connect(but, SIGNAL(clicked()), SLOT(apply()));
  hlay->addWidget(but);
  vlay->addLayout(hlay);
  setLayout(vlay);
  enableAll();
  autoResize();
  setAll(Adjustments());
}

SliderClipboard::~SliderClipboard() {
}

void SliderClipboard::autoResize() {
  resize(sa->widget()->sizeHint()
         + QSize(0, sizeHint().height() - sa->sizeHint().height())
         + sa->size() - sa->viewport()->contentsRect().size());
}  

Adjustments SliderClipboard::values() const {
  return val;
}

QSet<QString> SliderClipboard::mask() const {
  QSet<QString> msk;
  for (auto sli: jogs.keys())
    if (jogs[sli]->isChecked())
      msk.insert(sli);
  return msk;
}

void SliderClipboard::get(Adjustments *dest) const {
  for (auto sli: mask()) 
    dest->set(sli, val.get(sli));
}

void SliderClipboard::set(class Adjustments const &vv) {
  for (auto sli: mask()) {
    val.set(sli, vv.get(sli));
    double v = ControlSliders::sliderValue(vv, sli);
    labels[sli]->setText(QString::number(v, 'f', decimals(sli)));
  }
}

void SliderClipboard::setAll(class Adjustments const &vv) {
  val = vv;
  for (auto sli: labels.keys()) {
    double v = ControlSliders::sliderValue(vv, sli);
    labels[sli]->setText(QString::number(v, 'f', decimals(sli)));
  }
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
  labels[sli]->setEnabled(jogs[sli]->checkState());
}

int SliderClipboard::decimals(QString sli) {
  static QMap<QString, int> dec;
  if (!dec.contains(sli)) {
    auto info = SliderGroups::sliderGroups().sliderInfo(sli);
    if (info.mustep>=1)
      dec[sli] = 0;
    else
      dec[sli] = -floor(log10(info.mustep));
  }
  return dec[sli];
}
