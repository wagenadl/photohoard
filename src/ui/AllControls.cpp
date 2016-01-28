// AllControls.cpp

#include "AllControls.h"
#include "ControlGroup.h"
#include "GentleJog.h"
#include <QFile>
#include <QTextStream>
#include "PDebug.h"
#include <QBoxLayout>
#include <QSignalMapper>
#include "Adjustments.h"
#include <limits>
#include <QScrollBar>
#include <QEvent>
#include "SliderGroups.h"
#include "Action.h"

AllControls::AllControls(QWidget *parent): QScrollArea(parent) {
  QSignalMapper *mapper = new QSignalMapper(this);
  connect(mapper, SIGNAL(mapped(QString)), SLOT(sliderChange(QString)));
  QSignalMapper *nextmapper = new QSignalMapper(this);
  connect(nextmapper, SIGNAL(mapped(QString)), SLOT(goNext(QString)));
  QSignalMapper *prevmapper = new QSignalMapper(this);
  connect(prevmapper, SIGNAL(mapped(QString)), SLOT(goPrevious(QString)));

  SliderGroups sg;

  QWidget *w = new QWidget;//OneWayScroll;
  setWidget(w);
  setWidgetResizable(true);

  QVBoxLayout *vl = new QVBoxLayout;

  foreach (QString grp, sg.groups()) {
    ControlGroup *g = new ControlGroup(sg.groupLabel(grp));
    vl->addWidget(g);
    groups[grp] = g;

    foreach (QString sli, sg.sliders(grp)) {
      SliderGroups::SliderInfo const &info = sg.sliderInfo(sli);
      GentleJog *jog = new GentleJog(info.label);
      jog->setDefault(info.dflt);
      jog->setValue(info.dflt);
      jog->setMinimum(info.min);
      jog->setMaximum(info.max);
      jog->setMicroStep(info.mustep);
      jog->setSingleStep(info.step);
      jog->setPageStep(info.pgstep);
      jog->setMaxDelta(2*info.pgstep);
      if (info.mustep>=1)
	jog->setDecimals(0);
      else
	jog->setDecimals(-floor(log10(info.mustep)));
      jog->setToolTip(info.tip);

      connect(jog, SIGNAL(valueChanged(double)), mapper, SLOT(map()));
      connect(jog, SIGNAL(goPrevious()), prevmapper, SLOT(map()));
      connect(jog, SIGNAL(goNext()), nextmapper, SLOT(map()));
      mapper->setMapping(jog, sli);
      prevmapper->setMapping(jog, sli);
      nextmapper->setMapping(jog, sli);
      g->addWidget(jog);
      jogs[sli] = jog;
    }       
  }

  QString first, last;
  foreach (QString sli, sg.allSliders()) {
    previous[sli] = last;
    if (!last.isEmpty())
      next[last] = sli;
    if (first.isEmpty())
      first = sli;
    last = sli;
  }
  next[last] = first;
  previous[first] = last;

  vl->addStretch();
  
  w->setLayout(vl);

  for (auto cg: groups)
    cg->expand();

  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

AllControls::~AllControls() {
}

Adjustments const &AllControls::getAll() const {
  return adj;
}  

ControlGroup *AllControls::group(QString name) const {
  return groups.contains(name) ? groups[name] : 0;
}

GentleJog *AllControls::jog(QString name) const {
  return jogs.contains(name) ? jogs[name] : 0;
}

double AllControls::sliderValue(QString name) const {
  GentleJog *j = jog(name);
  ASSERT(j);
  return j->value();
}

void AllControls::setAll(Adjustments const &vv) {
  adj = vv;
  for (auto adjustmentname: vv.keys()) {
    double value = vv.get(adjustmentname);
    QString slidername = adjustmentname;
    GentleJog *j = jog(slidername);
    ASSERT(j); // hmmm.
    j->setValueQuietly(value);
  }
}
  
void AllControls::sliderChange(QString slidername) {
  // Must convert between slider names and adjustment names
  double value = sliderValue(slidername);
  QString adjustmentname = slidername;
  adj.set(adjustmentname, value);
  emit valueChanged(adjustmentname, value);
}

QSize AllControls::sizeHint() const {
  QWidget *vp = viewport();
  QWidget *wdg = widget();
  if (vp && wdg)
    return wdg->sizeHint() + size() - vp->contentsRect().size();
  else
    return QSize();
}

void AllControls::goNext(QString src) {
  QString dst = next[src];
  while (!dst.isEmpty() && dst!=src) {
    if (jogs.contains(dst) && jogs[dst]->isVisible()) {
      jogs[dst]->setFocus();
      ensureWidgetVisible(jogs[dst]);
      return;
    } else {
      dst = next[dst];
    }
  }
}

void AllControls::goPrevious(QString src) {
  QString dst = previous[src];
  while (!dst.isEmpty() && dst!=src) {
    if (jogs.contains(dst) && jogs[dst]->isVisible()) {
      jogs[dst]->setFocus();
      ensureWidgetVisible(jogs[dst]);
      return;
    } else {
      dst = previous[dst];
    }
  }
}

Actions const &AllControls::actions() {
  static Actions acts;
  if (acts.all().isEmpty()) {
    /* Make sure these Actions match the reality in GentleJog::keyPressEvent */
    acts
    << Action{"Left", "Decrease value (try Alt, Shift)"}
    << Action{"Right", "Increase value (try Alt, Shift)"}
    << Action{"Page Down", "Decrease value greatly"}
    << Action{"Page Up", "Increase value greatly"}
    << Action{"Up", "Move focus to slider above"}
    << Action{"Down", "Move focus to slider below"};
  }
  return acts;
}
