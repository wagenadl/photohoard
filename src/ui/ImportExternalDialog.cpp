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

ImportExternalDialog::ImportExternalDialog(Scanner *scanner, SessionDB *db,
                                           QList<QUrl> sources,
                                           QWidget *parent):
  QDialog(parent), scanner(scanner), db(db) {
  ui = new Ui_ImportExternalDialog;
  ui->setupUi(this);
  ui->ok->setEnabled(false);
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
  collector->cancel();
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

void ImportExternalDialog::completeCounts() {
  ui->ok->setEnabled(true);
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
  if (!dlg->exec()) {
    delete dlg;
    return;
  }
  
  QStringList images = dlg->collector->imageFiles();

  QStringList movies = dlg->ui->copyMovies->isChecked()
    ? dlg->collector->movieFiles() : QStringList();

  CopyIn::SourceDisposition disp = CopyIn::Leave;
  switch (dlg->ui->disposition->currentIndex()) {
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

  CopyIn *copyin = new CopyIn(scanner);
  connect(copyin, SIGNAL(completed(int,int)),
          dlg, SLOT(nowImport()));
  connect(copyin, SIGNAL(completed(int,int)),
          copyin, SLOT(deleteLater()));

  copyin->setDestination(dlg->ui->destination->text());
  if (dlg->ui->copyMovies->isChecked())
    copyin->setMovieDestination(dlg->ui->movieDestination->text());

  copyin->setSources(images);
  copyin->setMovieSources(movies);

  copyin->setSourceDisposition(disp);

  copyin->start();
}
