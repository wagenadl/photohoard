// FilterDialog.cpp

#include "FilterDialog.h"
#include "PDebug.h"
#include "ui_FilterDialog.h"
#include "Filter.h"
#include "SessionDB.h"
#include "Tags.h"
#include "TagDialog.h"
#include "Dialog.h"

FilterDialog::FilterDialog(SessionDB *db, QWidget *parent):
  QDialog(parent), db(db) {

  starting = true;
  ui = new Ui_FilterDialog();
  ui->setupUi(this);
  starting = false;
  Dialog::ensureSize(this);
}

void FilterDialog::prepCombos() {
  /* Get collections, cameras, etc. from db-> */
  prepCollections();
  prepCameras();
  prepFolderTree();
}

void FilterDialog::prepCameras() {
  prepMakes();
  prepModels();
  prepLenses();
}

void FilterDialog::prepMakes() {
  ui->cMake->clear();
  ui->cMake->addItem("Any make");

  QSqlQuery q = db->query("select distinct make from cameras");
  QStringList makes;
  while (q.next()) 
    makes << q.value(0).toString();
  q.finish();
  
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

  QSqlQuery q = make.isEmpty()
    ? db->query("select distinct camera from cameras")
    : db->query("select distinct camera from cameras where make=:a", make);
  QStringList models;
  while (q.next())
    models << q.value(0).toString();
  q.finish();
  
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
    q = db->query("select lens from lenses");
  } else {
    QString qq = "select distinct lenses.lens from lenses "
      "inner join photos on lenses.id==photos.lens "
      "inner join cameras on photos.camera==cameras.id where ";
    if (model.isEmpty()) {
      qq += "cameras.make==:a";
      q = db->query(qq, make);
    } else if (make.isEmpty()) {
      qq += "cameras.camera==:a";
      q = db->query(qq, model);
    } else {
      qq += "cameras.make==:a and cameras.camera==:b";
      q = db->query(qq, make, model);
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
  q.finish();
  
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
  ui->collectionBox->addItem("None");

  QStringList cc = tags.collections();
  if (cc.isEmpty())
    return;

  ui->cMake->insertSeparator(1);
  for (QString c: cc)
    ui->collectionBox->addItem(c);
}

Filter FilterDialog::extract() const {
  Filter f(db);
  f.setCollection(ui->collectionBox->currentIndex()==0 ? ""
                  : ui->collectionBox->currentText());
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
	      ui->sRejected->isChecked(),
	      ui->sNew->isChecked());
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

  QTreeWidgetItem *fl = ui->location->currentItem();
  if (fl)
    f.setFileLocation(fl->text(1));
  else
    f.unsetFileLocation();
  if (!ui->fileLocation->isChecked())
    f.unsetFileLocation();

  f.setTags(splitTags());
  if (!ui->tags->isChecked())
    f.unsetTags();
  return f;
}

QStringList FilterDialog::splitTags() const {
  QStringList res;
  for (auto s: ui->tagEditor->toPlainText().split(QRegExp("[,;&\n]+"))) {
    s = s.simplified();
    if (!s.isEmpty())
      res << s;
  }
  return res;
}

void FilterDialog::populate() {
  Filter f(db);
  f.loadFromDb();
  starting = true;
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
  ui->sNew->setChecked(f.statusNewImport());

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
  QList<QTreeWidgetItem *> its = ui->location->findItems(f.fileLocation(),
                                                         Qt::MatchExactly, 0);
  if (!its.isEmpty())
    ui->location->setCurrentItem(its.first(), 0,
                                 QItemSelectionModel::SelectCurrent);
  
  // Tags
  ui->tags->setChecked(f.hasTags());
  ui->tagEditor->setText(f.tags().join("\n"));

  recount();
  starting = false;
}

void FilterDialog::recount() {
  Filter f = extract();
  ui->count->setText(QString::number(f.count()));
}

void FilterDialog::setMaker() {
  if (starting)
    return;
  QString make = ui->cMake->currentIndex()==0 ? ""
    : ui->cMake->currentText();
  prepModels(make);
  // this will _cause_ setCamera to be called and hence force a recount.
}

void FilterDialog::setCamera() {
  if (starting)
    return;
  QString make = ui->cMake->currentIndex()==0 ? ""
    : ui->cMake->currentText();
  QString model = ui->cCamera->currentIndex()==0 ? ""
    : ui->cCamera->currentText();
  prepLenses(make, model);
  recount();
}

void FilterDialog::recolorTags() {
  if (starting)
    return;
  QStringList tt = splitTags();
  ui->tagInterpretation->setText(Tags(db).interpretation(tt));
  recount();
}

void FilterDialog::showEvent(QShowEvent *e) {
  populate();
  QDialog::showEvent(e);
}

void FilterDialog::buttonClick(QAbstractButton *b) {
  QDialogButtonBox::ButtonRole role = ui->buttonBox->buttonRole(b);
  switch (role) {
  case QDialogButtonBox::AcceptRole:
  case QDialogButtonBox::ApplyRole:
    extract().saveToDb();
    emit applied();
    break;
  case QDialogButtonBox::ResetRole:
    Filter(db).saveToDb();
    populate();
    break;
  case QDialogButtonBox::RejectRole:
    // cancel
  default:
    break;
  }
}

void FilterDialog::browseFolders() {
  /* We need to consider all the roots directories in the database.
     Ideally, I'd like a modified QFileDialog that allows selection of
     those roots.
     And I want to start in the folder specified in the "location" editor.
     
   */
}

void FilterDialog::browseTags() {
  TagDialog *td = new TagDialog(db, true);
  int res = td->exec();
  if (res == QDialog::Accepted) {
    QString tag = Tags(db).smartName(td->terminalTag());
    QStringList taglist = splitTags();
    if (!tag.isEmpty())
      taglist << tag;
    ui->tagEditor->setText(taglist.join("\n"));
  }
  delete td;
}

void FilterDialog::buildTree(QTreeWidgetItem *parentit, quint64 parentid) {
  QSqlQuery q = db->query("select pathname, leafname, id from folders"
                         " where parentfolder==:a order by pathname",
                         parentid);
  while (q.next()) {
    QString path = q.value(0).toString();
    QString leaf = q.value(1).toString();
    quint64 id = q.value(2).toULongLong();
    QStringList cc; cc << leaf << path;
    QTreeWidgetItem *it = new QTreeWidgetItem(cc);
    parentit->addChild(it);
    buildTree(it, id);
  }
}  
  

void FilterDialog::prepFolderTree() {
  ui->location->clear();

  QSqlQuery q = db->query("select pathname, id from folders"
                         " where parentfolder is null order by pathname");
  while (q.next()) {
    QString path = q.value(0).toString();
    quint64 id = q.value(1).toULongLong();
    QStringList cc; cc << path << path;
    QTreeWidgetItem *it = new QTreeWidgetItem(cc);
    ui->location->addTopLevelItem(it);
    buildTree(it, id);
  }
}

void FilterDialog::selectAllLabels() {
  bool all = ui->cRed->isChecked()
    &&  ui->cYellow->isChecked()
    &&  ui->cGreen->isChecked()
    &&  ui->cBlue->isChecked()
    &&  ui->cPurple->isChecked();
  ui->cNone->setChecked(false);
  ui->cRed->setChecked(!all);
  ui->cYellow->setChecked(!all);
  ui->cGreen->setChecked(!all);
  ui->cBlue->setChecked(!all);
  ui->cPurple->setChecked(!all);
}
    
