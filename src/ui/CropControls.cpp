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
private:
  void addModeButtons(CropControls *);
  void addOrientButtons(CropControls *);
  void addAspects(CropControls *);
  void addSliders(CropControls *);
public:
  QMap<CropMode, QAbstractButton *> modeControls;
  QMap<Orient, QAbstractButton *> orientControls;
  QMap<QString, QAbstractButton *> aspectControls;
  QMap<QString, double> aspectValues;
  QMap<QString, GentleJog *> sliders;
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

  QStringList jogs;
  jogs << "Left" << "Right"
       << "Top" << "Bottom"
       << "Top left" << "Top right"
       << "Bottom left" << "Bottom right";
  for (QString s: jogs) {
    GentleJog *gj = new GentleJog(s);
    gj->setRange(0, 1000);
    gj->setDecimals(0);
    gj->setSingleStep(10);
    gj->setPageStep(100);
    gj->setMicroStep(1);
    gj->setMaxDelta(50);
    sliders[s] = gj;
    lay->addWidget(gj);
  }

  QObject::connect(sliders["Left"], SIGNAL(valueChanged(double)),
                   cc, SLOT(slideLeft(double)));
  QObject::connect(sliders["Right"], SIGNAL(valueChanged(double)),
                   cc, SLOT(slideRight(double)));
  QObject::connect(sliders["Top"], SIGNAL(valueChanged(double)),
                   cc, SLOT(slideTop(double)));
  QObject::connect(sliders["Bottom"], SIGNAL(valueChanged(double)),
                   cc, SLOT(slideBottom(double)));
  QObject::connect(sliders["Top left"], SIGNAL(valueChanged(double)),
                   cc, SLOT(slideTL(double)));
  QObject::connect(sliders["Top right"], SIGNAL(valueChanged(double)),
                   cc, SLOT(slideTR(double)));
  QObject::connect(sliders["Bottom left"], SIGNAL(valueChanged(double)),
                   cc, SLOT(slideBL(double)));
  QObject::connect(sliders["Bottom right"], SIGNAL(valueChanged(double)),
                   cc, SLOT(slideBR(double)));
}

void CropControlsUi::reflectAspect(QString a0) {
  for (QString a: aspectControls.keys())
    aspectControls[a]->setChecked(a==a0);
}

//////////////////////////////////////////////////////////////////////


CropControls::CropControls(QWidget *parent): QFrame(parent) {
  ui = new CropControlsUi;
  ui->populate(this);
  calc = new CropCalc;
  setAutoFillBackground(true);
  QPalette p = palette();
  p.setColor(QPalette::Window, QColor(160, 160, 160));
  //  setPalette(p);
}

CropControls::~CropControls() {
  delete ui;
  delete calc;
}

void CropControls::slideLeft(double) { }
void CropControls::slideRight(double) { }
void CropControls::slideTop(double) { }
void CropControls::slideBottom(double) { }
void CropControls::slideTL(double) { }
void CropControls::slideTR(double) { }
void CropControls::slideBL(double) { }
void CropControls::slideBR(double) { }

void CropControls::setAll(Adjustments const &, QSize) { }
void CropControls::setValue(QString, double) { }

void CropControls::toggleMode() { }
void CropControls::toggleOrient() { }
void CropControls::gotoSlider(int) { }
void CropControls::clickAspect(QString) { }
