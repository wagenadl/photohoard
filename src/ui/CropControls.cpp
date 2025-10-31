// CropControls.cpp

#include "CropControls.h"
#include <math.h>
#include "InstaSlot.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>
#include "GentleJog.h"
#include <QRectF>
#include <QVector>
#include <QMap>
#include "CropCalc.h"
#include "BoldButton.h"
#include <QSignalMapper>
#include <QDebug>
#include "ControlGroup.h"

//////////////////////////////////////////////////////////////////////
class CropControlsUi {
public:
  void populate(CropControls *);
  void reflectAspect(QString);
  void reflectLimits(CropCalc *);
private:
  void addModeButtons(CropControls *);
  void addOrientButtons(CropControls *);
  void addAspects(CropControls *);
  void addSliders(CropControls *);
  void addTools(CropControls *);
public:
  enum class Slider { Left=0, Right, Top, Bottom, TL, TR, BL, BR };
  QMap<CropMode, QAbstractButton *> modeControls;
  QMap<Orient, QAbstractButton *> orientControls;
  QMap<QString, QAbstractButton *> aspectControls;
  QMap<QString, double> aspectValues;
  QMap<Slider, GentleJog *> sliders;
  QLineEdit *customAspect;
  QVBoxLayout *vLayout;
};

void CropControlsUi::populate(CropControls *cc) {
  vLayout = new QVBoxLayout;
  vLayout->setContentsMargins(2, 2, 2, 2);
  cc->widget()->setLayout(vLayout);
  addModeButtons(cc);
  addOrientButtons(cc);
  addTools(cc);
  addAspects(cc);
  addSliders(cc);
  vLayout->addSpacing(100);
}

void CropControlsUi::addTools(CropControls *cc) {

  QHBoxLayout *lay = new QHBoxLayout();
  lay->setContentsMargins(0, 1, 0, 1);
  lay->addSpacing(1);

  auto *b = new QPushButton("Optimize");
  QObject::connect(b, SIGNAL(clicked()), cc, SLOT(optimize()));
  lay->addWidget(b);

  lay->addSpacing(1);
  QWidget *w = new QWidget();
  w->setLayout(lay);
  vLayout->addWidget(w);
}

void CropControlsUi::addModeButtons(CropControls *cc) {
  modeControls[CropMode::Free] = new BoldButton("Free");
  modeControls[CropMode::Aspect] = new BoldButton("Aspect");

  for (CropMode cm: modeControls.keys()) {
    modeControls[cm]->setAutoExclusive(true);
    QObject::connect(modeControls[cm], SIGNAL(toggled(bool)),
                     cc, SLOT(toggleMode()));
  }
  modeControls[CropMode::Free]->setChecked(true);

  QHBoxLayout *lay = new QHBoxLayout();
  lay->setContentsMargins(0, 1, 0, 1);
  lay->addSpacing(1);
  lay->addWidget(modeControls[CropMode::Free]);
  lay->addWidget(modeControls[CropMode::Aspect]);
  lay->addSpacing(1);
  QWidget *w = new QWidget();
  w->setLayout(lay);
  vLayout->addWidget(w);
}

void CropControlsUi::addOrientButtons(CropControls *cc) {
  orientControls[Orient::Auto] = new BoldButton("Auto");
  orientControls[Orient::Landscape] = new BoldButton("Landscape");
  orientControls[Orient::Portrait] = new BoldButton("Portrait");
  for (Orient o: orientControls.keys()) {
    orientControls[o]->setAutoExclusive(true);
    QObject::connect(orientControls[o], SIGNAL(toggled(bool)),
                     cc, SLOT(toggleOrient()));
  }
  
  QHBoxLayout *lay = new QHBoxLayout();
  lay->setContentsMargins(0, 1, 0, 1);
  lay->addSpacing(1);
  lay->addWidget(orientControls[Orient::Auto]);
  lay->addWidget(orientControls[Orient::Landscape]);
  lay->addWidget(orientControls[Orient::Portrait]);
  lay->addSpacing(1);
  QWidget *w = new QWidget();
  w->setLayout(lay);
  vLayout->addWidget(w);
}

void CropControlsUi::addAspects(CropControls *cc) {
  int row = 0;
  int col = 0;
  QGridLayout *lay = new QGridLayout();
  lay->setContentsMargins(0, 1, 0, 1);
  QSignalMapper *sigmap = new QSignalMapper(cc);
  QObject::connect(sigmap, SIGNAL(mappedString(QString)),
                   cc, SLOT(clickAspect(QString)));
  
  auto nextRow = [&]() {
    if (col>0) {
      col = 0;
      row ++;
    }
  };
  auto nextColumn = [&]() {
    col ++;
  };
  auto addLabeled = [&](QString lbl, double v) {
    BoldButton *bb = new BoldButton(lbl);
    lay->addWidget(bb, row, col);
    nextColumn();
    QObject::connect(bb, SIGNAL(clicked()),
                     sigmap, SLOT(map()));
    sigmap->setMapping(bb, lbl);
    aspectControls[lbl] = bb;
    aspectValues[lbl] = v;
  };
  auto addAspect = [&](int a, int b) {
    addLabeled(QString("%1:%2").arg(a).arg(b), a*1./b);
  };
  
  addAspect(1,1);
  addLabeled("Custom", 1);
  customAspect = new QLineEdit();
  lay->addWidget(customAspect, row, col, 1, -1);
  nextRow();

  addAspect(5,4);
  addAspect(4,3);
  addLabeled(QString::fromUtf8("5:3½"), 5/3.5);
  nextRow();

  addAspect(3,2);
  addAspect(16,10);
  addAspect(16,9);
  nextRow();

  addLabeled("Letter", 11./8.5);
  addLabeled("A4", 297./210);
  addLabeled("Golden", sqrt(5.)/2 + 1./2);
  nextRow();

  QObject::connect(customAspect, SIGNAL(textEdited(QString const &)),
                   cc, SLOT(customChanged()));
  QObject::connect(customAspect, SIGNAL(returnPressed()),
                   cc, SLOT(customConfirmed()));

  QWidget *w = new QWidget;
  w->setLayout(lay);
  vLayout->addWidget(w);
}

void CropControlsUi::addSliders(CropControls *cc) {
  //QWidget *container = new QWidget();
  auto *container = new ControlGroup("Crop");
  vLayout->addWidget(container);
//  QVBoxLayout *lay = new QVBoxLayout;
//  lay->setContentsMargins(0, 1, 0, 1);
//  container->setLayout(lay);

  sliders[Slider::Left] = new GentleJog("Left");
  sliders[Slider::Right] = new GentleJog("Right");
  sliders[Slider::Top] = new GentleJog("Top");
  sliders[Slider::Bottom] = new GentleJog("Bottom");
  sliders[Slider::TL] = new GentleJog("Top left");
  sliders[Slider::TR] = new GentleJog("Top right");
  sliders[Slider::BL] = new GentleJog("Bottom left");
  sliders[Slider::BR] = new GentleJog("Bottom right");

  for (int i=0; i<8; i++) {
    Slider s = Slider(i);
    Slider nxt = Slider((i+1)&7);
    Slider prv = Slider((i-1)&7);
    QObject::connect(sliders[s], SIGNAL(goNext()),
                     sliders[nxt], SLOT(setFocus()));
    QObject::connect(sliders[s], SIGNAL(goPrevious()),
                     sliders[prv], SLOT(setFocus()));
    container->addWidget(sliders[s]);
  }
  
  for (GentleJog *gj: sliders) {
    gj->setRange(0, 1000);
    gj->setDecimals(0);
    gj->setSingleStep(10);
    gj->setPageStep(100);
    gj->setMicroStep(1);
    gj->setMaxDelta(50);
  }

  QObject::connect(sliders[Slider::Left], SIGNAL(valueChanged(double)),
                   cc, SLOT(slideLeft(double)));
  QObject::connect(sliders[Slider::Right], SIGNAL(valueChanged(double)),
                   cc, SLOT(slideRight(double)));
  QObject::connect(sliders[Slider::Top], SIGNAL(valueChanged(double)),
                   cc, SLOT(slideTop(double)));
  QObject::connect(sliders[Slider::Bottom], SIGNAL(valueChanged(double)),
                   cc, SLOT(slideBottom(double)));
  QObject::connect(sliders[Slider::TL], SIGNAL(valueChanged(double)),
                   cc, SLOT(slideTL(double)));
  QObject::connect(sliders[Slider::TR], SIGNAL(valueChanged(double)),
                   cc, SLOT(slideTR(double)));
  QObject::connect(sliders[Slider::BL], SIGNAL(valueChanged(double)),
                   cc, SLOT(slideBL(double)));
  QObject::connect(sliders[Slider::BR], SIGNAL(valueChanged(double)),
                   cc, SLOT(slideBR(double)));

  container->expand();
}

void CropControlsUi::reflectAspect(QString a0) {
  for (QString a: aspectControls.keys())
    aspectControls[a]->setChecked(a==a0);
  if (a0.isEmpty())
    modeControls[CropMode::Free]->setChecked(true);
  else
    modeControls[CropMode::Aspect]->setChecked(true);
}

void CropControlsUi::reflectLimits(CropCalc *calc) {
  Adjustments const &adj = calc->adjustments();

  sliders[Slider::Left]->setMaximum(calc->pseudoSliderMaxLeft());
  sliders[Slider::Right]->setMaximum(calc->pseudoSliderMaxRight());
  sliders[Slider::Top]->setMaximum(calc->pseudoSliderMaxTop());
  sliders[Slider::Bottom]->setMaximum(calc->pseudoSliderMaxBottom());
  sliders[Slider::TL]->setMaximum(calc->pseudoSliderMaxTL());
  sliders[Slider::TR]->setMaximum(calc->pseudoSliderMaxTR());
  sliders[Slider::BL]->setMaximum(calc->pseudoSliderMaxBL());
  sliders[Slider::BR]->setMaximum(calc->pseudoSliderMaxBR());
  
  sliders[Slider::Left]->setValueQuietly(adj.cropl);
  sliders[Slider::Right]->setValueQuietly(adj.cropr);
  sliders[Slider::Top]->setValueQuietly(adj.cropt);
  sliders[Slider::Bottom]->setValueQuietly(adj.cropb);
  sliders[Slider::TL]->setValueQuietly(calc->pseudoSliderValueTL());
  sliders[Slider::TR]->setValueQuietly(calc->pseudoSliderValueTR());
  sliders[Slider::BL]->setValueQuietly(calc->pseudoSliderValueBL());
  sliders[Slider::BR]->setValueQuietly(calc->pseudoSliderValueBR());
  
}

//////////////////////////////////////////////////////////////////////


CropControls::CropControls(QWidget *parent): QScrollArea(parent) {
  QWidget *w = new QWidget;//OneWayScroll;
  setWidget(w);
  setWidgetResizable(true);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  calc = new CropCalc;
  ui = new CropControlsUi;
  ui->populate(this);
  ui->reflectLimits(calc);
  setAutoFillBackground(true);
}

CropControls::~CropControls() {
  delete ui;
  delete calc;
}

void CropControls::slideLeft(double v) {
  calc->slideLeft(v);
  reflectAndEmit();
}

void CropControls::slideRight(double v) {
  calc->slideRight(v);
  reflectAndEmit();
}

void CropControls::slideTop(double v) {
  calc->slideTop(v);
  reflectAndEmit();
}

void CropControls::slideBottom(double v) {
  calc->slideBottom(v);
  reflectAndEmit();
}

void CropControls::slideTL(double v) {
  calc->slideTL(v);
  reflectAndEmit();
}

void CropControls::slideTR(double v) {
  calc->slideTR(v);
  reflectAndEmit();
}

void CropControls::slideBL(double v) {
  calc->slideBL(v);
  reflectAndEmit();
}

void CropControls::slideBR(double v) {
  calc->slideBR(v);
  reflectAndEmit();
}

void CropControls::setAll(Adjustments const &adj, QSize osize1) {
  osize = osize1;
  setAll(adj);
}

void CropControls::setAll(Adjustments const &adj) {
  calc->reset(adj, osize);
  ui->modeControls[CropMode::Free]->setChecked(true);
  ui->orientControls[Orient::Auto]->setChecked(true);
  ui->reflectLimits(calc);
  ui->reflectAspect("");
}

void CropControls::setValue(QString k, double v) {
  calc->setValue(k, v);
  ui->modeControls[CropMode::Free]->setChecked(true);
  ui->orientControls[Orient::Auto]->setChecked(true);
  ui->reflectLimits(calc);
  ui->reflectAspect(""); 
}

void CropControls::toggleMode() {
  if (ui->modeControls[CropMode::Free]->isChecked()) {
    if (calc->cropMode() != CropMode::Free) {
      calc->setFree();
      ui->reflectAspect("");
      reflectAndEmit();
    }
  } else if (ui->modeControls[CropMode::Aspect]->isChecked()) {
    if (calc->cropMode() != CropMode::Aspect) {
      calc->setFixedAspect();
      reflectAndEmit();
    }
  }
}

void CropControls::reflectAndEmit() {
  ui->reflectLimits(calc);
  emit rectangleChanged(calc->cropRect(), calc->originalSize());
}  

void CropControls::toggleOrient() {
  if (ui->orientControls[Orient::Landscape]->isChecked()) {
    if (calc->aspectRatio()<1) {
      calc->setOrient(Orient::Landscape);
      reflectAndEmit();
    }
  } else if (ui->orientControls[Orient::Portrait]->isChecked()) {
    if (calc->aspectRatio()>1) {
      calc->setOrient(Orient::Portrait);
      reflectAndEmit();
    }
  } 
}

static double interpretCustom(QString s, bool *ok) {
  int idx = s.indexOf(":");
  double a = 0;
  if (idx<0)
    idx = s.indexOf("x");
  if (idx>0) {
    a = s.left(idx).toDouble(ok);
    if (ok)
      a /= s.mid(idx+1).toDouble(ok);
  } else {
    a = s.toDouble(ok);
  }
  if (ok)
    *ok = a>0.1 && a<10;
  return a;
}

void CropControls::clickAspect(QString s) {
  Orient o = Orient::Auto;
  for (Orient k: ui->orientControls.keys())
    if (ui->orientControls[k]->isChecked())
      o = k;
  if (s=="Custom") {
    bool ok;
    ui->aspectValues[s] = interpretCustom(ui->customAspect->text(), &ok);
    if (!ok)
      ui->aspectValues[s] = 1;
  }
  calc->setAspect(ui->aspectValues[s], o);
  ui->reflectAspect(s);
  reflectAndEmit();
}

void CropControls::optimize() {
  calc->optimize();
  reflectAndEmit();
}

void CropControls::customChanged() {
  ui->modeControls[CropMode::Free]->setChecked(true);
}

void CropControls::customConfirmed() {
  clickAspect("Custom");
}

QSize CropControls::sizeHint() const {
  QWidget *vp = viewport();
  QWidget *wdg = widget();
  if (vp && wdg)
    return wdg->sizeHint() + size() - vp->contentsRect().size();
  else
    return QSize();
}
