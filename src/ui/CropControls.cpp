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

//////////////////////////////////////////////////////////////////////
class CropControlsUi {
public:
  void populate(CropControls *);
  void reflectAspect(QString);
  void reflectLimits(CropCalc *);
  void reflectCustom(CropCalc *);
private:
  void addModeButtons(CropControls *);
  void addOrientButtons(CropControls *);
  void addAspects(CropControls *);
  void addSliders(CropControls *);
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
  cc->setLayout(vLayout);
  addModeButtons(cc);
  addOrientButtons(cc);
  addAspects(cc);
  addSliders(cc);
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
  QObject::connect(sigmap, SIGNAL(mapped(QString)),
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
  nextRow();

  addAspect(5,4);
  addAspect(4,3);
  addAspect(7,5);
  nextRow();

  addAspect(3,2);
  addAspect(16,10);
  addAspect(16,9);
  nextRow();

  addLabeled("Letter", 11./8.5);
  addLabeled("A4", 297./210);
  addLabeled("Golden", sqrt(5.)/2 + 1./2);
  nextRow();

  addLabeled("Custom", 1);

  customAspect = new QLineEdit();
  lay->addWidget(customAspect, row, col, 1, -1);

  QWidget *w = new QWidget;
  w->setLayout(lay);
  vLayout->addWidget(w);
}

void CropControlsUi::addSliders(CropControls *cc) {
  QWidget *container = new QWidget();
  vLayout->addWidget(container);
  QVBoxLayout *lay = new QVBoxLayout;
  lay->setContentsMargins(0, 1, 0, 1);
  container->setLayout(lay);

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
    lay->addWidget(sliders[s]);
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
}

void CropControlsUi::reflectAspect(QString a0) {
  for (QString a: aspectControls.keys())
    aspectControls[a]->setChecked(a==a0);
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

void CropControlsUi::reflectCustom(CropCalc *calc) {
  customAspect->setText(QString::number(calc->aspectRatio()));
}


//////////////////////////////////////////////////////////////////////


CropControls::CropControls(QWidget *parent): QFrame(parent) {
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

void CropControls::slideRight(double) { }
void CropControls::slideTop(double) { }
void CropControls::slideBottom(double) { }
void CropControls::slideTL(double) { }
void CropControls::slideTR(double) { }
void CropControls::slideBL(double) { }
void CropControls::slideBR(double) { }

void CropControls::setAll(Adjustments const &adj, QSize osize) {
  calc->reset(adj, osize);
  ui->modeControls[CropMode::Free]->setChecked(true);
  ui->orientControls[Orient::Auto]->setChecked(true);
  ui->reflectAspect("");
}

void CropControls::setValue(QString k, double v) {
  calc->setValue(k, v);
  ui->modeControls[CropMode::Free]->setChecked(true);
  ui->orientControls[Orient::Auto]->setChecked(true);
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
      calc->setAspect();
      reflectAndEmit();
    }
  }
}

void CropControls::reflectAndEmit() {
  ui->reflectLimits(calc);
  ui->reflectCustom(calc);
  emit rectangleChanged(calc->cropRect(), calc->originalSize());
}  

void CropControls::toggleOrient() {
  if (ui->orientControls[Orient::Landscape]->isChecked()) {
    if (calc->aspectRatio()<1) {
      calc->setAspect(Orient::Landscape);
      reflectAndEmit();
    }
  } else if (ui->orientControls[Orient::Portrait]->isChecked()) {
    if (calc->aspectRatio()>1) {
      calc->setAspect(Orient::Portrait);
      reflectAndEmit();
    }
  } 
}

void CropControls::clickAspect(QString s) {
  Orient o = Orient::Auto;
  for (Orient k: ui->orientControls.keys())
    if (ui->orientControls[k]->isChecked())
      o = k;
  if (s=="Custom") {
    bool ok;
    ui->aspectValues[s] = ui->customAspect->text().toDouble(&ok);
    if (!ok || ui->aspectValues[s]<.1)
      ui->aspectValues[s] = 1;
  }
  calc->setAspect(ui->aspectValues[s], o);
  reflectAndEmit();
}
