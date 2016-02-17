// CropControls.cpp

#include "CropControls.h"
#include <math.h>
#include "InstaSlot.h"

CropControls::CropControls(QWidget *parent): QWidget(parent) {
  populate();
  connectAll();
}

void CropControls::populate() {
  setLayout(new QVBoxLayout);
  addModeButtons();
  addOrientButtons();
  addAspects();
  addSliders();
}

void CropControls::addModeButtons() {
  addButtons(modeControls,
             QStringList() << "Free" << "Aspect" << "Size");
  for (int k=0; k<3; k++) {
    auto *pb = modeControls[k];
    new InstaBool(pb, SIGNAL(toggled(bool)),
                  this, [pb, k, this](bool b) {
                    QFont f = pb->font();
                    f->setWeight(b ? QFont::Bold : QFont::Normal);
                    pb->setFont(f);
                    this->setMode(CropControls::Mode(k));
                  });
  }
}


QGridLayout *CropControls::addGrid() {
  QWidget *container = new QWidget();
  layout()->addWidget(container);
  QGridLayout *lay = new QGridLayout();
  container->setLayout(lay);
  return lay;

void CropControls::addOrientButtons() {
  QGridLayout *lay = addGrid();
  conta
  addButtons(orientControls,
             QStringList() << "Auto" << "Landscape" << "Portrait");
  for (int k=0; k<3; k++) {
    auto *pb = orientControls[k];
    new InstaBool(pb, SIGNAL(toggled(bool)),
                  this, [pb, k, this](bool b) {
                    QFont f = pb->font();
                    f->setWeight(b ? QFont::Bold : QFont::Normal);
                    pb->setFont(f);
                    this->setOrientation(CropControls::Orientation(k));
                  });
  }
}

void CropControls::addButtons(QVector<class QAbstractButton *> &lst,
                              QStringList lbls) {
  QWidget *container = new QWidget();
  layout()->addWidget(container);
  QGridLayout *lay = new QGridLayout();
  container->setLayout(lay);
  int k=0;
  for (QString s: lbls) {
    QPushButton *pb = new QPushButton(s);
    pb->setCheckable(true);
    pb->setAutoExclusive(true);
    if (k==0)
      pb->setChecked(true);
    container->layout()->addWidget(pb, k++);
  }
}

void CropControls::addAspects() {
  aspectLayout = 0;
  
  addAspect(1,1);
  addAspect(5,4);
  addAspect(4,3);
  addAspect(7,5);
  addAspect(3,2);
  addAspect(16,10);
  addAspect(16,9);
  addAspect("Letter", 11./8.5);
  addAspect("A4", 297./210);
  addAspect("Golden", sqrt(5.)/2 + 1./2);
  aspectControls << "Custom";
  
  for (int i=0; i<aspectControls.size(); i++) {
    auto *pb =  aspectControls[i];
    new InstaBool(pb, SIGNAL(toggled(bool)),
                  [this, pb, i](bool b) {
                    QFont f = pb->font();
                    f->setWeight(b ? QFont::Bold : QFont::Normal);
                    pb->setFont(f);
                    if (b) {
                      for (auto *p1, this->aspectControls)
                        if (p1!=pb)
                          p1->setChecked(false);
                      this->setAspectIndex(i);
                    }
                  });
  }    
}

void CropControls::addAspect(int a, int b) {
  addAspect(QString("%1:%2").arg(a).arg(b), a*1./b);
}

void CropControls::addAspect(QString lbl, double v) {
  constexpr int COLS = 4;
  static int row;
  static int col;
  if (!aspectLayout) {
    aspectLayout = new QGridLayout();
    QWidget *container = new QWidget();
    layout()->addWidget(container);
    container->setLayout(aspectLayout);
    row = 0;
    col = 0;
  }
  aspectValues << v;
  QPushButton *pb = new QPushButton(lbl);
  pb->setCheckable(true);
  aspectLayout->addWidget(pb, col, row);
  col++;
  if (col>=COLS) {
    row++;
    col=0;
  }
}

void CropControls::addSliders() {
  QWidget *container = new QWidget();
  layout->addWidget(container);
  QVBoxLayout *lay = new QVBoxLayout();
  container->setLayout(lay);

  QStringList jogs << "Left" << "Right"
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
    sliders << gj;
    lay->addWidget(gj);
}
