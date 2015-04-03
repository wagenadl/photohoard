// FilterDialog.cpp

#include "FilterDialog.h"
#include "PDebug.h"
#include "ui_FilterDialog.h"
#include "Filter.h"
#include "PhotoDB.h"
#include "NoResult.h"
#include "Tags.h"

FilterDialog::FilterDialog(PhotoDB const &db, QWidget *parent):
  QDialog(parent), db(db) {
  ui = new Ui_FilterDialog();
  ui->setupUi(this);
}

void FilterDialog::prepCombos() {
  /* Get collections, cameras, etc. from db. */
  prepCollections();
  prepCameras();
}

void FilterDialog::prepCameras() {
  prepMakes();
  prepModels();
  prepLenses();
}

void FilterDialog::prepMakes() {
  ui->cMake->clear();
  ui->cMake->addItem("Any make");

  QSqlQuery q = db.query("select distinct make from cameras");
  QStringList makes;
  while (q.next()) 
    makes << q.value(0).toString();
  qSort(makes);
  makes.removeOne("");

  if (!makes.isEmpty())
    ui->cMake->insertSeparator(1);

  for (QString m: makes)
    ui->cMake->addItem(m);
  
}

void FilterDialog::prepModels(QString make) {
  QString current = ui->cCamera->currentText();
  ui->cCamera->clear();
  ui->cCamera->addItem("Any model");

  QSqlQuery q = make.isEmpty() ? db.query("select distinct camera from cameras")
    : db.query("select distinct camera from cameras where make=:a", make);
  QStringList models;
  while (q.next())
    models << q.value(0).toString();
  qSort(models);
  models.removeOne("");

  if (!make.isEmpty() || models.size()<20) {
    if (!models.isEmpty())
      ui->cCamera->insertSeparator(1);
  
    for (QString m: models)
      ui->cCamera->addItem(m);
    if (models.size()==1) {
      ui->cCamera->setCurrentIndex(2);
      ui->cCamera->setEnabled(false);
    } else {
      int idx = ui->cCamera->findText(current);
      if (idx>=0)
        ui->cCamera->setCurrentIndex(idx);
      ui->cCamera->setEnabled(true);    
    }
    ui->cCamera->setMaxVisibleItems(models.size()+2);
  } else {
    ui->cCamera->setItemText(0, "Select make first");
    ui->cCamera->setEnabled(false);
  }
}

void FilterDialog::prepLenses(QString make, QString model) {
  QString current = ui->cLens->currentText();
  ui->cLens->clear();
  ui->cLens->addItem("Any lens");

  QSqlQuery q;
  if (make.isEmpty() && model.isEmpty()) {
    q = db.query("select lens from lenses");
  } else {
    QString qq = "select distinct lenses.lens from lenses "
      "inner join photos on lenses.id==photos.lens "
      "inner join cameras on photos.camera==cameras.id where ";
    if (model.isEmpty()) {
      qq += "cameras.make==:a";
      q = db.query(qq, make);
    } else if (make.isEmpty()) {
      qq += "cameras.camera==:a";
      q = db.query(qq, model);
    } else {
      qq += "cameras.make==:a and cameras.camera==:b";
      q = db.query(qq, make, model);
    }
  }

  QStringList lenses;
  while (q.next()) {
    QString l = q.value(0).toString();
    if (l>="0" && l<="99999") {
      // ignore numeric lenses
    } else {
      lenses << l;
    }
  }
  qSort(lenses);
  lenses.removeOne("");

  if (!model.isEmpty() || lenses.size()<20) {
    if (!lenses.isEmpty())
      ui->cLens->insertSeparator(1);
  
    for (QString m: lenses)
      ui->cLens->addItem(m);
  
    if (lenses.size()==0) {
      ui->cLens->setItemText(0, "Fixed lens");
      ui->cLens->setEnabled(false);
    } else if (lenses.size()==1) {
      ui->cLens->setCurrentIndex(2);
      ui->cLens->setEnabled(false);
    } else {
      int idx = ui->cLens->findText(current);
      if (idx>=0)
        ui->cLens->setCurrentIndex(idx);
      ui->cLens->setEnabled(true);
    }
    ui->cLens->setMaxVisibleItems(lenses.size()+2);
  } else {
    ui->cLens->setItemText(0, "Select make or model first");
    ui->cLens->setEnabled(false);
  }
}

void FilterDialog::prepCollections() {
  Tags tags(db);

  ui->collectionBox->clear();
  ui->collectionBox->addItem("Any");

  int coltag = tags.findOne("Collections");
  if (coltag>0) {
    QSet<QString> cols;
    QSqlQuery q = db.query("select tag from tags "
			   "where parent==:a", coltag);
    while (q.next())
      cols << q.value(0).toString();
    if (!cols.isEmpty())
      ui->cMake->insertSeparator(1);
    for (QString c: cols)
      ui->collectionBox->addItem(c);
  }
}

Filter FilterDialog::extract() const {
  Filter f;
  f.setCollection(ui->collectionBox->currentText());
  if (!ui->collection->isChecked())
    f.unsetCollection();

  QSet<PhotoDB::ColorLabel> cc;
  if (ui->cNone->isChecked())
    cc << PhotoDB::ColorLabel::None;
  if (ui->cRed->isChecked())
    cc << PhotoDB::ColorLabel::Red;
  if (ui->cYellow->isChecked())
    cc << PhotoDB::ColorLabel::Yellow;
  if (ui->cGreen->isChecked())
    cc << PhotoDB::ColorLabel::Green;
  if (ui->cBlue->isChecked())
    cc << PhotoDB::ColorLabel::Blue;
  if (ui->cPurple->isChecked())
    cc << PhotoDB::ColorLabel::Purple;
  f.setColorLabels(cc);
  if (!ui->colorLabel->isChecked()) 
    f.unsetColorLabels();

  f.setStarRating(ui->rMinBox->value(), ui->rMaxBox->value());
  if (!ui->starRating->isChecked())
    f.unsetStarRating();

  f.setStatus(ui->sAccepted->isChecked(),
	       ui->sUnset->isChecked(),
	       ui->sRejected->isChecked());
  if (!ui->status->isChecked())
    f.unsetStatus();

  f.setCamera(ui->cMake->currentIndex()>0
	       ? ui->cMake->currentText() : QString(""),
	       ui->cCamera->currentIndex()>0
	       ? ui->cCamera->currentText() : QString(""),
	       ui->cLens->currentIndex()>0
	       ? ui->cLens->currentText() : QString(""));
  if (!ui->camera->isChecked())
    f.unsetCamera();

  f.setDateRange(ui->dStart->date(), ui->dEnd->date());
  if (!ui->dateRange->isChecked())
    f.unsetDateRange();
  
  f.setFileLocation(ui->location->text());
  if (!ui->fileLocation->isChecked())
    f.unsetFileLocation();

  f.setTags(ui->tagEditor->toPlainText().split(QRegExp("[, ]+")));
  if (!ui->tags->isChecked())
    f.unsetTags();
  return f;
}

void FilterDialog::populate(Filter const &f) {
  prepCombos();

  // Collections
  ui->collection->setChecked(f.hasCollection());
  int idx = ui->collectionBox->findText(f.collection());
  ui->collectionBox->setCurrentIndex(idx>0 ? idx : 0);

  // Color labels
  ui->colorLabel->setChecked(f.hasColorLabels());
  ui->cNone->setChecked(f.includesColorLabel(PhotoDB::ColorLabel::None));
  ui->cRed->setChecked(f.includesColorLabel(PhotoDB::ColorLabel::Red));
  ui->cYellow->setChecked(f.includesColorLabel(PhotoDB::ColorLabel::Yellow));
  ui->cGreen->setChecked(f.includesColorLabel(PhotoDB::ColorLabel::Green));
  ui->cBlue->setChecked(f.includesColorLabel(PhotoDB::ColorLabel::Blue));
  ui->cPurple->setChecked(f.includesColorLabel(PhotoDB::ColorLabel::Purple));

  // Star rating
  ui->starRating->setChecked(f.hasStarRating());
  ui->rMinSlider->setValue(f.minStarRating());
  ui->rMaxSlider->setValue(f.maxStarRating());

  // Status
  ui->status->setChecked(f.hasStatus());
  ui->sAccepted->setChecked(f.statusAccepted());
  ui->sRejected->setChecked(f.statusRejected());
  ui->sUnset->setChecked(f.statusUnset());

  // Camera
  ui->camera->setChecked(f.hasCamera());
  idx = ui->cMake->findText(f.cameraMake());
  ui->cMake->setCurrentIndex(idx>1 ? idx : 0);
  idx = ui->cCamera->findText(f.cameraModel());
  ui->cCamera->setCurrentIndex(idx>1 ? idx : 0);
  idx = ui->cLens->findText(f.cameraLens());
  ui->cLens->setCurrentIndex(idx>1 ? idx : 0);

  // Date range
  ui->dateRange->setChecked(f.hasDateRange());
  ui->dStart->setDate(f.startDate());
  ui->dEnd->setDate(f.endDate());

  // File location
  ui->fileLocation->setChecked(f.hasFileLocation());
  ui->location->setText(f.fileLocation());
  
  // Tags
  ui->tags->setChecked(f.hasTags());
  ui->tagEditor->setText(f.tags().join(", "));

  recount();
}

void FilterDialog::recount() {
  Filter f = extract();
  try {
    ui->count->setText(QString::number(f.count(db)));
  } catch (QSqlQuery const &q) {
    pDebug() << "recount error: " + q.lastError().text()
      + " from " + q.lastQuery();
  } catch (NoResult) {
    pDebug() << "recount error: no result";
  } catch (...) {
    pDebug() << "recount error";
  }
}

void FilterDialog::setMaker() {
  QString make = ui->cMake->currentIndex()==0 ? ""
    : ui->cMake->currentText();
  prepModels(make);
  // this will _cause_ setCamera to be called and hence force a recount.
}

void FilterDialog::setCamera() {
  QString make = ui->cMake->currentIndex()==0 ? ""
    : ui->cMake->currentText();
  QString model = ui->cCamera->currentIndex()==0 ? ""
    : ui->cCamera->currentText();
  prepLenses(make, model);
  recount();
}

void FilterDialog::recolorTags() {
  pDebug() << "FD::recolorTags";
  recount();
}

void FilterDialog::showEvent(QShowEvent *e) {
  populate(f0);
  QDialog::showEvent(e);
}

void FilterDialog::buttonClick(QAbstractButton *b) {
  QDialogButtonBox::ButtonRole role = ui->buttonBox->buttonRole(b);
  switch (role) {
  case QDialogButtonBox::AcceptRole:
  case QDialogButtonBox::ApplyRole:
    f0 = extract();
    emit apply();
    break;
  case QDialogButtonBox::ResetRole:
    f0 = Filter();
    populate(f0);
    break;
  case QDialogButtonBox::RejectRole:
    // cancel
  default:
    break;
  }
}
