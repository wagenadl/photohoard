// ImportGUI.cpp

#include "ImportGUI.h"
#include "ImportJob.h"
#include "ImportExternalDialog.h"
#include "ImportOtherUserDialog.h"
#include "ImportLocalDialog.h"
#include "PDebug.h"
#include <QProgressDialog>
#include "Tags.h"
#include "SessionDB.h"

ImportGUI::ImportGUI(class SessionDB *db,
                     class Scanner *scanner,
                     QList<QUrl> const &sources,
                     QObject *parent): QObject(parent) {
  extDlg = 0;
  othUserDlg = 0;
  locDlg = 0;
  progressDlg = 0;
  
  job = new ImportJob(db, scanner, sources, this);

  connect(job, SIGNAL(complete(QString)),
          SLOT(finishUpCompletedJob(QString)));
  connect(job, SIGNAL(canceled()),
          SLOT(cleanUpCanceledJob()));
}

ImportGUI::~ImportGUI() {
  delete extDlg;
  delete othUserDlg;
  delete locDlg;
  delete job;
}

void ImportGUI::showAndGo() {
  if (job->sourceInfo()->isExternalMedia()) {
    job->setOperation(ImportJob::Operation::Import);
    job->setAutoCollection();
    extDlg = new ImportExternalDialog(job, Tags(job->database()).collections());
    job->countSources();
    connect(extDlg, SIGNAL(accepted()), SLOT(dlgAccept()));
    connect(extDlg, SIGNAL(canceled()), SLOT(dlgCancel()));
    extDlg->show();
  } else if (
}

void ImportGUI::finishUpCompletedJob(QString errmsg) {
  qDebug() << "finishUpCompletedJob" << errmsg;
  deleteLater();
}

void ImportGUI::cleanUpCanceledJob() {
  qDebug() << "cleanUpCanceledJob";
  deleteLater();
}

void ImportGUI::dlgAccept() {
  if (extDlg) {
    job->setCollection(extDlg->collection());
    job->setDestination(extDlg->destination());
    job->setSourceDisposition(extDlg->sourceDisposition());
    if (extDlg->hasMovieDestination())
      job->setMovieDestination(extDlg->movieDestination());
    else
      job->setNoMovieDestination();
    extDlg->hide();
  } else {
    CRASH("No dialog");
  }

  if (job->operation()==ImportJob::Operation::Import) {
    progressDlg = new QProgressDialog("Copying...", "Cancel",
                                      0, job->preliminarySourceCount());
    connect(progressDlg, SIGNAL(canceled()), job, SLOT(cancel()));
    connect(job, SIGNAL(countsUpdated(int, int)),
            progressDlg, SLOT(setMaximum(int)));
    connect(job, SIGNAL(progress(int)), progressDlg, SLOT(setValue(int)));
  }

  job->authorize();
}

void ImportGUI::dlgCancel() {
  deleteLater();
}
