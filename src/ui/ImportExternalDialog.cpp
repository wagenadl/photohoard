// ImportExternalDialog.cpp

#include "ImportExternalDialog.h"
#include "Collector.h"
#include "SessionDB.h"
#include "ui_ImportExternalDialog.h"
#include "Tags.h"
#include "Filter.h"
#include "CopyIn.h"
#include "PDebug.h"
#include "Scanner.h"
#include <QProgressDialog>

ImportExternalDialog::ImportExternalDialog(Scanner *scanner, SessionDB *db,
                                           QList<QUrl> sources,
                                           QWidget *parent):
  QWidget(parent), scanner(scanner), db(db) {
  progress = 0;
  copyin = 0;
  ui = new Ui_ImportExternalDialog;
  ui->setupUi(this);
  accept_prov = false;
  complete_cnt = false;
  what = ui->what->text();
  movieWhat = ui->copyMovies->text();
  ui->movieContainer->hide();
  
  collector = new Collector(this);
  connect(collector, SIGNAL(progress(int,int)), SLOT(updateCounts(int,int)));
  connect(collector, SIGNAL(complete()), SLOT(completeCounts()));
  collector->collect(sources);

  Tags tags(db);
  QStringList collections = tags.collections();
  ui->collection->clear();
  for (auto s: collections)
    ui->collection->addItem(s);

  QString dest = "";
  
  Filter filter(db);
  filter.loadFromDb();
  if (filter.hasCollection()) {
    QString coll = filter.collection();
    int idx = ui->collection->findText(coll);
    if (idx>=0)
      ui->collection->setCurrentIndex(idx);
    int tagid = tags.findCollection(coll);
    if (tagid>0) {
      QSqlQuery q
        = db->constQuery("select pathname from folders"
                         " inner join defaulttags"
                         " on folders.id==defaulttags.folder"
                         " where tag==:a"
                         " order by length(pathname) limit 1",
                         tagid);
      if (q.next())
        dest = q.value(0).toString();
    }
  }
  if (dest.isEmpty())
    dest = db->simpleQuery("select pathname from folders"
                           " order by length(pathname) limit 1").toString();

  ui->destination->setText(CopyIn::autoDest(dest + "/photohoard"));
}

ImportExternalDialog::~ImportExternalDialog() {
  if (progress)
    delete progress;
}

void ImportExternalDialog::changeCollection(QString coll) {
  Tags tags(db);
  int tagid = tags.findCollection(coll);
  if (tagid>0) {
    QSqlQuery q
      = db->constQuery("select pathname from folders"
                       " inner join defaulttags"
                       " on folders.id==defaulttags.folder"
                       " where tag==:a"
                       " order by length(pathname) limit 1",
                       tagid);
    if (q.next())
      ui->destination->setText(CopyIn::autoDest(q.value(0).toString()
                                                + "/photohoard"));
  }
}  

void ImportExternalDialog::updateCounts(int nimg, int nmov) {
  ui->what->setText(what.arg(nimg).arg(nimg==1 ? "" : ""));
  ui->copyMovies->setText(what.arg(nmov).arg(nmov==1 ? "" : ""));
  if (nmov)
    ui->movieContainer->show();
}

void ImportExternalDialog::cancel() {
  close();
  if (copyin)
    copyin->cancel();
  if (progress)
    progress->deleteLater();
  deleteLater();
}

void ImportExternalDialog::completeCounts() {
  complete_cnt = true;
  if (accept_prov)
    startCopy();
}

void ImportExternalDialog::allowImport() {
  accept_prov = true;
  if (complete_cnt)
    startCopy();
  else
    hide();
}

void ImportExternalDialog::nowImport() {
  qDebug() << "ImportExternal: nowImport";
  scanner->addTree(ui->destination->text(), ui->collection->currentText());
  deleteLater();
}

void ImportExternalDialog::showAndGo(Scanner *scanner,
                                     SessionDB *db,
                                     QList<QUrl> sources) {
  ImportExternalDialog *dlg = new ImportExternalDialog(scanner, db, sources);
  dlg->show();
}

void ImportExternalDialog::startCopy() {
  hide();
  QStringList images = collector->imageFiles();
  QStringList movies = ui->copyMovies->isChecked()
    ? collector->movieFiles() : QStringList();

  collector->deleteLater();
  collector = 0;

  progress = new QProgressDialog("Copying...", "Cancel",
                                 0, images.size() + movies.size());
  progress->setValue(0);
  connect(progress, SIGNAL(canceled()), SLOT(cancel()));
  
  CopyIn::SourceDisposition disp = CopyIn::Leave;
  switch (ui->disposition->currentIndex()) {
  case 0:
    disp = CopyIn::Backup;
    break;
  case 1:
    disp = CopyIn::Leave;
    break;
  default:
    qDebug() << "Unknown disposition";
    break;
  }

  copyin = new CopyIn(this);
  connect(copyin, SIGNAL(completed(int,int)),
          SLOT(nowImport()));
  connect(copyin, SIGNAL(progress(int)),
          progress, SIGNAL(setValue()));
  
  copyin->setDestination(ui->destination->text());
  if (ui->copyMovies->isChecked())
    copyin->setMovieDestination(ui->movieDestination->text());

  copyin->setSources(images);
  copyin->setMovieSources(movies);

  copyin->setSourceDisposition(disp);

  copyin->start();
}
