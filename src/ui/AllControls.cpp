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
  ASSERT(groups.contains(name));
  return groups[name];
}

GentleJog *AllControls::jog(QString name) const {
  ASSERT(jogs.contains(name));
  return jogs[name];
}

static double const *wbsli2adj() {
  static double mm[9]{
    1., .62, -.05,
    1., -.34, .85,
    1., .46, -.27
  };
  return mm;
}

static double const *wbadj2sli() {
  static double mm[9]{
    -0.84234, 0.40653, 1.43581,
     3.15315, -0.61937, -2.53378,
     2.25225,  0.45045,  -2.70270
  };
  return mm;
}

void AllControls::setAll(Adjustments const &a) {
  static QSet<QString> specials;
  if (specials.isEmpty()) 
    specials << "nlcontrast" << "nlcontrastscale"
	     << "expose" << "wb" << "wbg";

  adj = a;

  QMap<QString, double> vv;
  for (auto k: adj.keys())
    if (!specials.contains(k))
      vv[k] = adj.get(k);

  double nlc = adj.get("nlcontrast");
  double nlcs = adj.get("nlcontrastscale");
  double nlc_loc = pow(nlcs / 0.9, 2);
  double nlc_s = nlc * pow(1.0-nlcs, 0.75);
  vv["nlc_strength"] =  nlc_s;
  vv["nlc_localness"] = nlc_loc;

  double ex = adj.get("expose");
  double wb = ex + adj.get("wb")/5;
  double wg = ex + adj.get("wbg")/15;
  double const *mm = wbadj2sli();
  vv["expose"] = mm[0]*ex + mm[1]*wb + mm[2]*wg;
  vv["wbg"] = mm[3]*ex + mm[4]*wb + mm[5]*wg;
  vv["wb"] = mm[6]*ex + mm[7]*wb + mm[8]*wg;
  
  for (auto k:vv.keys())
    jog(k)->setValueQuietly(vv[k]);
}

void AllControls::sliderChange(QString slider) {
  double value = jog(slider)->value();
  if (slider=="nlc_strength" || slider=="nlc_localness") {
    double nlc_s = jog("nlc_strength")->value();
    double nlc_loc = jog("nlc_localness")->value();
    double nlcs = 0.9 * pow(nlc_loc, 0.5);
    double nlc = nlc_s / pow(1.0-nlcs, 0.75);
    setAndEmit("nlcontrastscale", nlcs);
    setAndEmit("nlcontrast", nlc);
  } else if (slider=="expose" || slider=="wb" || slider=="wbg") {
    double E = jog("expose")->value();
    double W = jog("wb")->value();
    double G = jog("wbg")->value();
    double const *mm = wbsli2adj();
    double ex = mm[0]*E + mm[1]*G + mm[2]*W;
    double wb = mm[3]*E + mm[4]*G + mm[5]*W;
    double wg = mm[6]*E + mm[7]*G + mm[8]*W;
    setAndEmit("expose", ex);
    setAndEmit("wb", 5*(wb-ex));
    setAndEmit("wbg", 15*(wg-ex));
  } else {
    setAndEmit(slider, value);
  }
}

void AllControls::setAndEmit(QString k, double v) {
  adj.set(k, v);
  emit valueChanged(k, v);
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
