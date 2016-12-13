// ControlSliders.cpp

#include "ControlSliders.h"
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

ControlSliders::ControlSliders(bool ro, QWidget *parent): QScrollArea(parent) {
  QSignalMapper *mapper = new QSignalMapper(this);
  connect(mapper, SIGNAL(mapped(QString)), SLOT(sliderChange(QString)));
  QSignalMapper *nextmapper = new QSignalMapper(this);
  connect(nextmapper, SIGNAL(mapped(QString)), SLOT(goNext(QString)));
  QSignalMapper *prevmapper = new QSignalMapper(this);
  connect(prevmapper, SIGNAL(mapped(QString)), SLOT(goPrevious(QString)));

  SliderGroups const &sg(SliderGroups::sliderGroups());

  QWidget *w = new QWidget;//OneWayScroll;
  if (ro)
    w->setEnabled(false);
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

ControlSliders::~ControlSliders() {
}

Adjustments const &ControlSliders::getAll() const {
  return adj;
}  

ControlGroup *ControlSliders::group(QString name) const {
  ASSERT(groups.contains(name));
  return groups[name];
}

GentleJog *ControlSliders::jog(QString name) const {
  ASSERT(jogs.contains(name));
  return jogs[name];
}

/* WHITE BALANCE AND EXPOSURE
   In a—failed—attempt to create perceptual uniformity of slider effects,
   I coded the LMS scaling in AdjusterXYZ as
      M ← M * 2^(ex)
      S ← S * 2^(ex + 1/5 wb)
      L ← L * 2^(ex + 1/15 wbg)
   where ex is the expose parameter, wb is the whitebal parameter, and wbg
   is the green adjust parameter.
   What I really want is a W parameter that changes blue <-> yellow and
   a GA parameter that changes green <-> red.
   I assume that Intensity ~= 3G + 2R + B (approximately; linear RGB to Lch
   maps R to L=53, G to L=87; B to L=32; there is a nonlinearity as well.)
   Thus for DeltaI = 0, I must have 3 DeltaG + 2 DeltaR + Delta B = 0.
   If I say that W causes DeltaB = W, then DeltaG = Delta R = W/5.
   Likewise, if I say that GA causes DeltaG = GA, then DeltaR = 3 GA/2.

   I want:
   
 (M)  [    ex       ]     [ E  ]
 (S)  [ex + 1/5 wb  ] = M [ GA ]
 (L)  [ex + 1/15 wbg]     [ W  ]

 Here is how to achieve that:
 
  rgb_w = [-.2 -.2 1];
  lms_w = colorconvert(rgb_w, 'from', 'linearrgb', 'to', 'lms');

  rgb_ga = [ -1 2/3 0];
  lms_ga = colorconvert(rgb_ga, 'from', 'linearrgb', 'to', 'lms');

  M = [1 1 1; lms_ga([2 3 1]); lms_w([2 3 1])]';
  iM = inv(M);

  if 1
  fprintf(1, "static double const *wbsli2adj() {\n");
  fprintf(1, "  static double mm[9]{\n");
  for k=1:3
    fprintf(1, "  %9.6f, %9.6f, %9.6f,\n",M(k,:));
  end
  fprintf(1, "  };\n");
  fprintf(1, "  return mm;\n");
  fprintf(1,"}\n\n");
  fprintf(1, "static double const *wbadj2sli() {\n");
  fprintf(1, "  static double mm[9]{\n");
  for k=1:3
    fprintf(1, "  %9.6f, %9.6f, %9.6f,\n",iM(k,:));
  end
  fprintf(1, "  };\n");
  fprintf(1, "  return mm;\n");
  fprintf(1,"}\n\n");
  end
 */
static double const *wbsli2adj() {
  static double mm[9]{
   1.000000,  0.347184, -0.079930,
   1.000000,  0.055257,  0.847499,
   1.000000,  0.112432, -0.144067,
  };
  return mm;
}

static double const *wbadj2sli() {
  static double mm[9]{
  -0.436672,  0.173537,  1.263135,
   4.193742, -0.271261, -3.922481,
   0.241815,  0.992864, -1.234679,
  };
  return mm;
}

// static double const *wbsli2adj() {
//   static double mm[9]{
//     1., .62, -.05,
//     1., -.34, .85,
//     1., .46, -.27
//   };
//   return mm;
// }
// 
// static double const *wbadj2sli() {
//   static double mm[9]{
//     -0.84234, 0.40653, 1.43581,
//      3.15315, -0.61937, -2.53378,
//      2.25225,  0.45045,  -2.70270
//   };
//   return mm;
// }

double ControlSliders::sliderValue(Adjustments const &a, QString slider) {
  if (slider=="nlc_strength") {
    double nlc = a.get("nlcontrast");
    double nlcs = a.get("nlcontrastscale");
    return nlc * pow(1.0-nlcs, 0.75);
  } else if (slider=="nlc_localness") {
    double nlcs = a.get("nlcontrastscale");
    return pow(nlcs / 0.9, 2);
  } else if (slider=="expose") {
    double ex = a.get("expose");
    double wb = ex + a.get("wb")/5;
    double wg = ex + a.get("wbg")/15;
    double const *mm = wbadj2sli();
    return mm[0]*ex + mm[1]*wb + mm[2]*wg;
  } else if (slider=="wbg") {
    double ex = a.get("expose");
    double wb = ex + a.get("wb")/5;
    double wg = ex + a.get("wbg")/15;
    double const *mm = wbadj2sli();
    return mm[3]*ex + mm[4]*wb + mm[5]*wg;
  } else if (slider=="wb") {
    double ex = a.get("expose");
    double wb = ex + a.get("wb")/5;
    double wg = ex + a.get("wbg")/15;
    double const *mm = wbadj2sli();
    return mm[6]*ex + mm[7]*wb + mm[8]*wg;
  } else {
    return a.get(slider);
  }
}  

void ControlSliders::setAll(Adjustments const &a) {

  adj = a;

  for (auto k: jogs.keys()) 
    jogs[k]->setValueQuietly(sliderValue(adj, k));
}

void ControlSliders::sliderChange(QString slider) {
  double value = jog(slider)->value();
  if (slider=="nlc_strength" || slider=="nlc_localness") {
    double nlc_s = jog("nlc_strength")->value();
    double nlc_loc = jog("nlc_localness")->value();
    double nlcs = 0.9 * pow(nlc_loc, 0.5);
    double nlc = nlc_s / pow(1.0-nlcs, 0.75);
    adj.set("nlcontrastscale", nlcs);
    adj.set("nlcontrast", nlc);
  } else if (slider=="expose" || slider=="wb" || slider=="wbg") {
    double E = jog("expose")->value();
    double W = jog("wb")->value();
    double G = jog("wbg")->value();
    double const *mm = wbsli2adj();
    double ex = mm[0]*E + mm[1]*G + mm[2]*W;
    double wb = mm[3]*E + mm[4]*G + mm[5]*W;
    double wg = mm[6]*E + mm[7]*G + mm[8]*W;
    adj.set("expose", ex);
    adj.set("wb", 5*(wb-ex));
    adj.set("wbg", 15*(wg-ex));
  } else {
    adj.set(slider, value);
  }
  emit valuesChanged();
}

QSize ControlSliders::sizeHint() const {
  QWidget *vp = viewport();
  QWidget *wdg = widget();
  if (vp && wdg)
    return wdg->sizeHint() + size() - vp->contentsRect().size();
  else
    return QSize();
}

void ControlSliders::goNext(QString src) {
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

void ControlSliders::goPrevious(QString src) {
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

Actions const &ControlSliders::actions() {
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

void ControlSliders::setLayer(int l) {
  ControlGroup *recomp = groups["recompose"];
  recomp->setEnabled(l==0);
}
