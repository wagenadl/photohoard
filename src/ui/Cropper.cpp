// Cropper.cpp

#include "Cropper.h"
#include "ui_Cropper.h"
#include "InstaSlot.h"
#include "Adjustments.h"
#include <QDebug>

Cropper::Cropper(QWidget *parent): QWidget(parent) {
  ui = new Ui_Cropper();
  ui->setupUi(this);
  connectRats();
}

void Cropper::setAll(Adjustments const &adj, QSize s) {
  origSize = s;
  cropRect = QRect(QPoint(adj.cropl, adj.cropt),
		   QPoint(s.width()-adj.cropr, s.height()-adj.cropb));
  ui->tgt_free->setChecked(true);
  reflectRect();
}

void Cropper::setValue(QString k, double v) {
  if (k=="cropl") 
    cropRect.setLeft(v);
  else if (k=="cropr")
    cropRect.setRight(origSize.width() - v);
  else if (k=="cropt")
    cropRect.setTop(v);
  else if (k=="cropb")
    cropRect.setBottom(origSize.height() - v);
  else
    return;
  
  ui->tgt_free->setChecked(true);
  reflectRect();
}

Cropper::RatioMode Cropper::ratioMode() const {
  if (ui->orient_auto->isChecked())
    return RatioMode::Auto;
  if (ui->orient_landscape->isChecked())
    return RatioMode::Landscape;
  if (ui->orient_portrait->isChecked())
    return RatioMode::Portrait;
  return RatioMode::Auto; // hmmm.
}

void Cropper::setRatioMode(Cropper::RatioMode r) {
  switch (r) {
  case RatioMode::Auto:
    ui->orient_auto->setChecked(true);
    break;
  case RatioMode::Portrait:
    ui->orient_portrait->setChecked(true);
    break;
  case RatioMode::Landscape:
    ui->orient_landscape->setChecked(true);
    break;
  }
}

void Cropper::connectRats() {
  QList<QAbstractButton *> modes;
  modes << ui->orient_auto
	<< ui->orient_portrait
	<< ui->orient_landscape;
  for (auto m: modes)
    new InstaBool(m, SIGNAL(toggled(bool)),
		  this, [m](bool b) {
		    QFont f(m->font());
		    f.setWeight(b ? QFont::Bold : QFont::Normal);
		    m->setFont(f);
		  });
  modes[2]->setChecked(true);
  modes[0]->setChecked(true);

  rats << ui->rat_1_1
       << ui->rat_5_4
       << ui->rat_4_3
       << ui->rat_3_2
       << ui->rat_7_5
       << ui->rat_16_10
       << ui->rat_16_9
       << ui->rat_297_210
       << ui->rat_110_85
       << ui->rat_1618_1000;
  int ratvals[]{
    1,1,
      5,4,
      4,3,
      3,2,
      7,5,
      16,10,
      16,9,
      297,210,
      110,85,
      1618,1000
      };
  
  for (int n=0; n<rats.size(); n++) {
    QAbstractButton *rat = rats[n];
    rat->setAutoExclusive(false);
    int va = ratvals[2*n];
    int vb = ratvals[2*n+1];
    new InstaBool(rat, SIGNAL(toggled(bool)),
		  this, [this, rat, va, vb](bool b) {
		    QFont f(rat->font());
		    f.setWeight(b ? QFont::Bold : QFont::Normal);
		    rat->setFont(f);
		    if (b) {
		      this->setRatio(va, vb);
		      this->uncheckRatsExcept(rat);
		    }
		  });
  }

  new InstaBool(ui->tgt_aspect, SIGNAL(toggled(bool)),
		this, [this](bool b) {
		  qDebug() << "aspect " << b;
		  if (!b)
		    this->uncheckRatsExcept(0);
		});
}

void Cropper::uncheckRatsExcept(QAbstractButton *b) {
  for (auto r: rats) 
    if (r!=b)
      r->setChecked(false);
}

void Cropper::setRatio(int va, int vb) {
  qDebug() << "setRatio" << va << vb;
  if (!ui->tgt_aspect->isChecked())
    ui->tgt_aspect->setChecked(true);
}

void Cropper::reflectRect() {
}
