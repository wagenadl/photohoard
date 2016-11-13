// PurgeDialog.cpp

#include "PurgeDialog.h"
#include "Purge.h"
#include "ui_PurgeDialog.h"
#include <QDebug>
#include <QKeyEvent>
#include <QMessageBox>
#include <QStyle>
#include "AutoCache.h"
#include "PhotoDB.h"
#include <QProgressDialog>
#include "RestoreDialog.h"
#include "Dialog.h"

PurgeDialog::PurgeDialog(PhotoDB *db, Purge const *purge):
  db(db), purge(purge) {
  int nsibv = purge->versionsWithSiblings().size();
  int nsibp = purge->photosWithOtherVersions().size();
  int nnosibv = purge->versionsWithoutSiblings().size();
  int nnosibp = purge->photosWithoutOtherVersions().size();
  int ndel = purge->photosThatCanBeDeleted().size();
  int norphans = purge->orphanedPhotos().size();
  int nordel = purge->orphansThatCanBeDeleted().size();

  ui = new Ui_PurgeDialog;
  ui->setupUi(this);
  ui->warningIcon->
    setPixmap(style()->standardIcon(QStyle::SP_MessageBoxWarning)
              .pixmap(32));
  ui->warningContainer->hide();

  if (nsibv>0) {
    ui->mainLabelShort->hide();
    QString txt = ui->mainLabelLong->text();
    txt.replace("#k", QString::number(nsibv));
    txt.replace("#s", nsibv==1 ? "" : "s");
    txt.replace("#n", QString::number(nsibp));
    txt.replace("#t", nsibp==1 ? "" : "s");
    ui->mainLabelLong->setText(txt);

    if (nnosibv>0) {
      QString txt = ui->purgeLabel->text();
      txt.replace("#m", QString::number(nnosibp));
      txt.replace("#s", nnosibp==1 ? "" : "s");
      ui->purgeLabel->setText(txt);
      if (ndel>0) {
        QString txt = ui->deleteLabel->text();
        txt.replace("#s", nnosibp==1 ? "" : "s");
        txt.replace("#those", nnosibp==1 ? "that" : "those");
        txt.replace("#q", nnosibp==1 ? "" : "s");
        ui->deleteLabel->setText(txt);
      } else {
        ui->deleteContainer->hide();
      }
    } else {
      ui->purgeContainer->hide();
      ui->deleteContainer->hide();
    }
  } else if (nnosibp>0) {
    ui->mainLabelLong->hide();
    QString txt = ui->mainLabelShort->text();
    txt.replace("#m", QString::number(nnosibp));
    txt.replace("#s", nnosibp==1 ? "" : "s");
    ui->mainLabelShort->setText(txt);
    ui->purgeContainer->hide();
    ui->purgeCheck->setChecked(true);
    if (ndel>0) {
      ui->deleteSpacer->hide();
      QString txt = ui->deleteLabel->text();
      txt.replace("#s", nnosibp==1 ? "" : "s");
      txt.replace("#those", nnosibp==1 ? "that" : "those");
      txt.replace("#q", nnosibp==1 ? "" : "s");
      ui->deleteLabel->setText(txt);
    } else {
      ui->deleteContainer->hide();
    }
  } else {
    ui->mainLabelLong->hide();
    ui->mainLabelShort->hide();    
    ui->purgeContainer->hide();
    ui->deleteContainer->hide();
  }

  if (norphans>0) {
    QString txt = ui->oldLabel->text();
    txt.replace("#are", norphans==1 ? "is" : "are");
    txt.replace("#q", QString::number(norphans));
    txt.replace("#s", norphans==1 ? "" : "s");
    ui->oldLabel->setText(txt);

    if (nordel>0) {
      QString txt = ui->deleteOldLabel->text();
      txt.replace("#s", norphans==1 ? "" : "s");
      txt.replace("#t", norphans==1 ? "" : "s");
      txt.replace("#those", norphans==1 ? "that" : "those");
      ui->deleteOldLabel->setText(txt);
    } else {
      ui->oldContainer->hide();
    }

    txt = ui->restoreButton->text();
    txt.replace("#s", norphans==1 ? "" : "s");
    ui->restoreButton->setText(txt);
  } else {
    ui->oldLabel->hide();
    ui->oldContainer->hide();
    ui->restoreButton->hide();
  }

  connect(ui->okButton, SIGNAL(clicked()), SLOT(accept()));
  connect(ui->cancelButton, SIGNAL(clicked()), SLOT(reject()));
  connect(ui->restoreButton, SIGNAL(clicked()), SLOT(restoreClicked()));
  connect(ui->purgeCheck, SIGNAL(toggled(bool)), SLOT(deleteToggled()));
  connect(ui->deleteCheck, SIGNAL(toggled(bool)), SLOT(deleteToggled()));
  connect(ui->deleteOldCheck, SIGNAL(toggled(bool)), SLOT(deleteToggled()));

  Dialog::ensureSize(this);
}

PurgeDialog::~PurgeDialog() {
}

void PurgeDialog::deleteToggled() {
  if ((ui->deleteCheck->isChecked() && ui->deleteCheck->isEnabled())
      || ui->deleteOldCheck->isChecked()) 
    ui->warningContainer->show();
  else
    ui->warningContainer->hide();
}

void PurgeDialog::restoreClicked() {
  hide();
  RestoreDialog::restoreDialog(db, purge->orphanedPhotos());
  reject();
}

void PurgeDialog::keyPressEvent(QKeyEvent *e) {
  switch (e->key()) {
  case Qt::Key_Return: case Qt::Key_Enter:
    accept();
    break;
  case Qt::Key_Escape:
    reject();
    break;
  default:
    break;
  }
}

void PurgeDialog::purgeDialog(class PhotoDB *db, class AutoCache *ac) {
  Purge purge(db, ac);
  if (purge.versionsWithSiblings().isEmpty()
      && purge.photosWithoutOtherVersions().isEmpty()
      && purge.orphanedPhotos().isEmpty()) {
    QMessageBox::information(0, "Photohoard",
                             "There are no rejects to purge");
    return;
  }

  PurgeDialog dlg(db, &purge);
  int res = dlg.exec();
  if (res==0)
    return;

  int N = purge.versionsWithSiblings().size();
  if (dlg.ui->purgeCheck->isChecked()) {
    N += purge.versionsWithoutSiblings().size();
    if (dlg.ui->deleteCheck->isChecked()) 
      N += purge.photosThatCanBeDeleted().size();
    if (dlg.ui->deleteOldCheck->isChecked()) 
      N += purge.orphansThatCanBeDeleted().size();
  }

  QProgressDialog progress;
  progress.setLabelText("Purging rejects");
  progress.setMinimum(0);
  progress.setMaximum(N);
  int n = 0;  
    
  for (auto vsn: purge.versionsWithSiblings()) {
    /* First, purge from cache, then remove from db.
       That way, we won't have junk remaining in cache.
    */
    ac->purge(vsn);
    db->deleteVersion(vsn);
    progress.setValue(++n);
    if (progress.wasCanceled())
      return;
  }

  if (dlg.ui->purgeCheck->isChecked()) {
    for (auto vsn: purge.versionsWithoutSiblings()) {
      ac->purge(vsn);
      db->deleteVersion(vsn);
    progress.setValue(++n);
    if (progress.wasCanceled())
      return;
    }
    if (dlg.ui->deleteCheck->isChecked()) {
      auto map = purge.photosThatCanBeDeleted();
      for (auto pht: map.keys()) {
        QFile fn(map[pht]);
        if (fn.remove())
          db->deletePhoto(pht);
        progress.setValue(++n);
        if (progress.wasCanceled())
          return;
      }
    }
    if (dlg.ui->deleteOldCheck->isChecked()) {
      auto map = purge.orphansThatCanBeDeleted();
      for (auto pht: map.keys()) {
        QFile fn(map[pht]);
        if (fn.remove())
          db->deletePhoto(pht);
        progress.setValue(++n);
        if (progress.wasCanceled())
          return;
      }
    }
  }
}

