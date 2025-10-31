// FilterDialog.cpp

#include "FilterDialog.h"
#include "PDebug.h"
#include "ui_FilterDialog.h"
#include "Filter.h"
#include "SessionDB.h"
#include "Tags.h"
#include "TagDialog.h"
#include "Dialog.h"
#include <algorithm>

QString const noneLabel = "(none)";

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
  QString make = ui->cMake->currentIndex()==0 ? ""
    : ui->cMake->currentText();
  prepModels(make);
  QString model = ui->cCamera->currentIndex()==0 ? ""
    : ui->cCamera->currentText();
  prepLenses(make, model);
}

void FilterDialog::prepMakes() {
  QStringList makes;
  { DBReadLock lock(db);
    QSqlQuery q = db->constQuery("select distinct make from cameras"
                                 " where make>'' order by make");
    while (q.next()) 
      makes << q.value(0).toString();
  }

  ui->cMake->clear();
  ui->cMake->addItem("Any make");
  if (makes.isEmpty())
    return;
  ui->cMake->insertSeparator(1);
  for (QString m: makes)
    ui->cMake->addItem(m);
}

void FilterDialog::prepModels(QString make) {
  QString current = ui->cCamera->currentText();

  QStringList models;
  { DBReadLock lock(db);
    QSqlQuery q = make.isEmpty()
      ? db->constQuery("select distinct camera from cameras"
                       " where camera>'' order by camera")
      : db->constQuery("select distinct camera from cameras"
                       " where make=:a and camera>'' order by camera",
                       make);
    while (q.next())
      models << q.value(0).toString();
  }

  ui->cCamera->clear();
  if (make.isEmpty() && models.size()>20) {
    ui->cCamera->addItem("Select make first");
    ui->cCamera->setEnabled(false);
    return;
  }
  ui->cCamera->addItem("Any model");
  ui->cCamera->setEnabled(true);
  if (models.isEmpty())
    return;
  ui->cCamera->insertSeparator(1);
  for (QString m: models)
    ui->cCamera->addItem(m);
  
  int idx = ui->cCamera->findText(current);
  if (idx>=0)
    ui->cCamera->setCurrentIndex(idx);
  else
    ui->cCamera->setCurrentIndex(0);
  ui->cCamera->setMaxVisibleItems(models.size()+2);
}

void FilterDialog::prepLenses(QString make, QString model) {
  QString current = ui->cLens->currentText();

  QStringList lenses;
  { DBReadLock lock(db);
    QSqlQuery q;
    QString ordr = " order by lenses.lens";
    if (make.isEmpty() && model.isEmpty()) {
      q = db->constQuery("select lens from lenses" + ordr);
    } else {
      QString qq = "select distinct lenses.lens from lenses"
        " inner join photos on lenses.id==photos.lens"
        " inner join cameras on photos.camera==cameras.id"
        " where lenses.lens>'' and";
      if (model.isEmpty()) {
        qq += " cameras.make==:a" + ordr;
        q = db->constQuery(qq, make);
      } else if (make.isEmpty()) {
        qq += " cameras.camera==:a" + ordr;
        q = db->query(qq, model);
      } else {
        qq += " cameras.make==:a and cameras.camera==:b" + ordr;
        q = db->query(qq, make, model);
      }
    }
    
    while (q.next()) {
      QString lens = q.value(0).toString();
      bool ok;
      lens.toInt(&ok);
      if (ok) {
        // ignore purely numeric lense names
      } else {
        lenses << lens;
      }
    }
  }

  ui->cLens->clear();
  if (model.isEmpty() && lenses.size()>20) {
    ui->cLens->addItem("Select make or model first");
    ui->cLens->setEnabled(false);
    return;
  }
  if (lenses.isEmpty() && !make.isEmpty() && !model.isEmpty()) {
    ui->cLens->addItem("Fixed lens");
    ui->cLens->setEnabled(false);
    return;
  }
  ui->cLens->addItem("Any lens");
  ui->cLens->setEnabled(true);
  ui->cLens->insertSeparator(1);
  for (QString m: lenses)
    ui->cLens->addItem(m);
  
  int idx = ui->cLens->findText(current);
  if (idx>=0)
    ui->cLens->setCurrentIndex(idx);
  else
    ui->cLens->setCurrentIndex(0);
  ui->cLens->setMaxVisibleItems(lenses.size()+2);
}

void FilterDialog::prepCollections() {
  Tags tags(db);

  ui->collectionBox->clear();
  ui->collectionBox->addItem(noneLabel);

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
  for (auto s: ui->tagEditor->toPlainText().split(QRegularExpression("[,;&\n]+"))) {
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
  prepModels(f.cameraMake());
  idx = ui->cCamera->findText(f.cameraModel());
  ui->cCamera->setCurrentIndex(idx>1 ? idx : 0);
  prepLenses(f.cameraMake(), f.cameraModel());
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

  starting = false;
  recount();
}

void FilterDialog::recount() {
  if (starting)
    return;
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
  QStringList paths, leaves;
  QList<quint64> ids;
  { DBReadLock lock(db);
    QSqlQuery q = db->constQuery("select pathname, leafname, id from folders"
                            " where parentfolder==:a order by pathname",
                            parentid);
    while (q.next()) {
      paths << q.value(0).toString();
      leaves << q.value(1).toString();
      ids << q.value(2).toULongLong();
    }
  }
  for (int n=0; n<ids.size(); n++) {
    QStringList cc; cc << leaves[n] << paths[n];
    QTreeWidgetItem *it = new QTreeWidgetItem(cc);
    parentit->addChild(it);
    buildTree(it, ids[n]);
  }
}  
  

void FilterDialog::prepFolderTree() {
  ui->location->clear();

  QStringList paths;
  QList<quint64> ids;
  { DBReadLock lock(db);
    QSqlQuery q = db->constQuery("select pathname, id from folders"
                            " where parentfolder is null order by pathname");
    while (q.next()) {
      paths << q.value(0).toString();
      ids << q.value(1).toULongLong();
    }
  }
  for (int n=0; n<ids.size(); n++) {
    QStringList cc; cc << paths[n] << paths[n];
    QTreeWidgetItem *it = new QTreeWidgetItem(cc);
    ui->location->addTopLevelItem(it);
    buildTree(it, ids[n]);
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
    
