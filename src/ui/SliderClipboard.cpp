// SliderClipboard.cpp

#include "SliderClipboard.h"
#include <QVBoxLayout>
#include "SliderGroups.h"
#include <QCheckBox>

SliderClipboard::SliderClipboard(QWidget *parent): QScrollArea(parent) {
  QVBoxLayout *vlay = new QVBoxLayout;
  SliderGroups sg;
  foreach (QString grp, sg.groups()) {
    QCheckBox *gc = new QCheckBox;
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
      vl->addWidget(sc);
    }
    gf->setLayout(vl);
    groupFrame[grp] = gf;
    vlay->addWidget(gf);
  }
  setLayout(vlay);

  enableAll();
  autoResize();  
}

SliderClipboard::~SliderClipboard() {
}

void SliderClipboard::autoResize() {
  resize(sizeHint());
}  

Sliders SliderClipboard::values() const {
  return val;
}

QSet<QString> SliderClipboard::mask() const {
  QSet<QString> msk;
  for (auto it=jogs.begin(); it!=jogs.end(); it++) 
    if (it.value()->isChecked())
      msk.insert(it.key());
  return msk;
}

void SliderClipboard::get(Sliders *dest) const {
  for (auto it=jogs.begin(); it!=jogs.end(); it++) 
    if (it.value()->isChecked())
      dest->set(it.key(), val.get(it.key()));
}

void SliderClipboard::set(class Sliders const &vv) {
  for (auto it=jogs.begin(); it!=jogs.end(); it++) 
    if (it.value()->isChecked())
      val.set(it.key(), vv.get(it.key()));
}

void SliderClipboard::setAll(class Sliders const &vv) {
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
    if (all) {
      groupControl[grp]->setCheckState(Qt::Checked);
      groupFrame[grp]->hide();
    } else if (any) {
      groupControl[grp]->setCheckState(Qt::PartiallyChecked);
      groupFrame[grp]->show();
    } else {
      groupControl[grp]->setCheckState(Qt::Unchecked);
      groupFrame[grp]->hide();
    }
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
    if (all) {
      groupControl[grp]->setCheckState(Qt::Checked);
      groupFrame[grp]->hide();
    } else if (any) {
      groupControl[grp]->setCheckState(Qt::PartiallyChecked);
      groupFrame[grp]->show();
    } else {
      groupControl[grp]->setCheckState(Qt::Unchecked);
      groupFrame[grp]->hide();
    }
 
}

void SliderClipboard::disable(QString name, bool off) {
  enable(name, !off);
}

void SliderClipboard::goNext(QString) {
}

void SliderClipboard::goPrevious(QString) {
}


