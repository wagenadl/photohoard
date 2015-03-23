// FilterDialog.cpp

#include "FilterDialog.h"
#include "PDebug.h"
#include "ui_FilterDialog.h"
#include "Filter.h"
#include "PhotoDB.h"
#include "NoResult.h"

FilterDialog::FilterDialog(PhotoDB const &db, QWidget *parent):
  QDialog(parent), db(db) {
  ui = new Ui_FilterDialog();
  ui->setupUi(this);
  prepCombos();
  populate(Filter());
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
  QSqlQuery q = db.query("select make from cameras");
  QSet<QString> makes;

  ui->cMake->reset();
  ui->cMake->addItem("Any make");
}

void FilterDialog::prepCollections() {
  Tags tags(db);

  ui->collectionBox->clear();
  ui->addItem("Any");

  int coltag = tags.find("Collections");
  if (coltag>0) {
    QSet<QString> cols;
    QSqlQuery q = db.query("select tag from tags "
			   "where parent==:a", coltag);
    while (q.next())
      cols << q.value(0).toString();
    for (QString c: cols)
      ui->addItem(c);
  }
}

Filter FilterDialog::extract() const {
  Filter f;
  f.setCollection(ui->collectionBox->currentText());
  if (!ui->collection->isChecked())
    f.unsetCollection();

  QSet<int> cc;
  if (ui->cNone->isChecked())
    cc << int(PhotoDB::ColorLabel::None);
  if (ui->cRed->isChecked())
    cc << int(PhotoDB::ColorLabel::Red);
  if (ui->cYellow->isChecked())
    cc << int(PhotoDB::ColorLabel::Yellow);
  if (ui->cGreen->isChecked())
    cc << int(PhotoDB::ColorLabel::Green);
  if (ui->cBlue->isChecked())
    cc << int(PhotoDB::ColorLabel::Blue);
  if (ui->cPurple->isChecked())
    cc << int(PhotoDB::ColorLabel::Purple);
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

  f.setCamera(ui->cMaker->currentIndex()>0
	       ? ui->cMaker->currentText() : QString(""),
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

void FilterDialog::populate(Filter const &) {

  recount();
}

void FilterDialog::recount() {
  Filter f = extract();
  pDebug() << f.joinClause();
  pDebug() << f.whereClause(db);
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
  pDebug() << "FD::setMaker";
  recount();
}

void FilterDialog::setCamera() {
  pDebug() << "FD::setCamera";
  recount();
}

void FilterDialog::recolorTags() {
  pDebug() << "FD::recolorTags";
  recount();
}

void FilterDialog::showEvent(QShowEvent *e) {
  //
  QDialog::showEvent(e);
}

void FilterDialog::buttonClicked(QAbstractButton *b) {
  QDialogButtonBox::ButtonRole role = ui->buttonBox->buttonRole(b);
  switch (role) {
  case QDialogButtonBox::AcceptRole:
  case QDialogButtonBox::ApplyRole:
    emit apply();
    break;
  case QDialogButtonBox::ResetRole:
    populate(Filter());
    break;
  default:
    break;
  }
}
