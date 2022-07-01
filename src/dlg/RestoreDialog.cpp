// RestoreDialog.cpp

#include "RestoreDialog.h"
#include "ui_RestoreDialog.h"
#include "PDebug.h"
#include "PhotoDB.h"
#include "Dialog.h"

RestoreDialog::RestoreDialog(int N, QWidget *parent): QDialog(parent) {
  ui = new Ui_RestoreDialog;
  ui->setupUi(this);

  QString txt = ui->mainLabel->text();
  txt.replace("#q", QString::number(N));
  txt.replace("#s", N==1 ? "" : "s");
  txt.replace("#were", N==1 ? "was" : "were");
  ui->mainLabel->setText(txt);

  txt = ui->markReject->text();
  txt.replace("#s", N==1 ? "" : "s");
  ui->markReject->setText(txt);

  txt = ui->markUndecided->text();
  txt.replace("#s", N==1 ? "" : "s");
  ui->markUndecided->setText(txt);

  txt = ui->colorCheck->text();
  txt.replace("#s", N==1 ? "" : "s");
  ui->colorCheck->setText(txt);

  Dialog::ensureSize(this);
}

RestoreDialog::~RestoreDialog() {
}

void RestoreDialog::restoreDialog(PhotoDB *db, QSet<quint64> const &photos) {
  RestoreDialog dlg(photos.size());
  int res = dlg.exec();
  if (res==0)
    return;

  Transaction t(db);
  pDebug() << "restore1";
  for (auto pht: photos) {
    quint64 folderid = db->simpleQuery("select folder from photos"
                                       " where id==:a", pht).toULongLong();
    PhotoDB::AcceptReject accrej = dlg.ui->markReject->isChecked()
      ? PhotoDB::AcceptReject::Reject
      : PhotoDB::AcceptReject::Undecided;
    PhotoDB::ColorLabel color = PhotoDB::ColorLabel::None;
    if (dlg.ui->colorCheck->isChecked()) {
      if (dlg.ui->red->isChecked())
        color = PhotoDB::ColorLabel::Red;
      else if (dlg.ui->yellow->isChecked())
        color = PhotoDB::ColorLabel::Yellow;
      else if (dlg.ui->green->isChecked())
        color = PhotoDB::ColorLabel::Green;
      else if (dlg.ui->blue->isChecked())
        color = PhotoDB::ColorLabel::Blue;
      else if (dlg.ui->purple->isChecked())
        color = PhotoDB::ColorLabel::Purple;
    }
      
    QSqlQuery q = db->query("insert into versions(photo, acceptreject, "
                            " colorlabel)"
                            " values(:a,:b,:c)", pht, int(accrej), int(color));
    quint64 vsn = q.lastInsertId().toULongLong();
    db->query("insert into appliedtags(tag, version)"
              " select tag, :a from defaulttags where folder==:b",
              vsn, folderid);
  }
  t.commit();
}
