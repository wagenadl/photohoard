// RedateDialog.cpp

#include "RedateDialog.h"
#include "PhotoDB.h"
#include "ui_RedateDialog.h"

QString nicedate(QDateTime dt) {
  return dt.toString("hh:mm on MMM d, yyyy");
}

RedateDialog::RedateDialog(class PhotoDB *db,
                           QList<quint64> versions, quint64 keyvsn,
                           bool isimport,
                           QWidget *parent):
  QDialog(parent), db(db) {
  ui = new Ui_RedateDialog;
  ui->setupUi(this);

  QSet<quint64> photos;
  {
    DBReadLock lock(db);
    for (auto vid: versions) {
      QSqlQuery q = db->constQuery("select photo from versions where id==:a", vid);
      if (q.next())
        photos << q.value(0).toUInt();
    }
  }
  quint64 keypid = db->photoFromVersion(keyvsn);
  dtkey = db->captureDate(keypid);
  QDateTime dtfirst = dtkey;
  QDateTime dtlast = dtkey;
  QString fn = db->photoRecord(keypid).filename;
  for (auto pid: photos) {
    QDateTime dt = db->captureDate(pid);
    olddtt[pid] = dt;
    if (dt < dtfirst)
      dtfirst = dt;
    else if (dt > dtlast)
      dtlast = dt;
  }
  ui->select1->hide();
  ui->select2->hide();
  ui->import1->hide();
  ui->import2->hide();
  if (photos.size() > 1) {
    // multiple photos in selection
    if (isimport) {
      ui->import2->show();
      ui->import2->setText(ui->import2->text()
                           .arg(nicedate(dtfirst)).arg(nicedate(dtlast)));
    } else {
      ui->select2->show();
      ui->select2->setText(ui->select2->text()
                           .arg(nicedate(dtfirst)).arg(nicedate(dtlast)));
    }
    ui->particular->show();
    ui->particular->setText(ui->particular->text()
                            .arg(fn).arg(nicedate(dtkey)));
  } else {
    // single photo
    if (isimport) {
      ui->import1->show();
      ui->import1->setText(ui->import1->text()
                           .arg(fn).arg(nicedate(dtkey)));
    } else {
        ui->select1->show();
        ui->select1->setText(ui->select1->text()
                                .arg(fn).arg(nicedate(dtkey)));
    }
    ui->particular->hide();
    ui->othernote->hide();
  }
  ui->newdate->setDateTime(dtkey);
}

RedateDialog::~RedateDialog() {
}

void RedateDialog::apply() {
  // do the work
  {
    DBWriteLock lock(db);
    int newsec = ui->newdate->dateTime().toSecsSinceEpoch();
    int oldsec = dtkey.toSecsSinceEpoch();
    int delta = newsec - oldsec;
    for (quint64 pid: olddtt.keys()) {
      QDateTime newdt = QDateTime::fromSecsSinceEpoch(olddtt[pid].toSecsSinceEpoch() + delta);
      db->query("update photos set capturedate=:a where id==:b",
                newdt.toString(Qt::ISODate), pid);
    }
  }
  // and clean up
  deleteLater();
  close();
  emit applied();
}
