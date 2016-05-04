// ImportGUI.cpp

#include "ImportGUI.h"
#include "ImportJob.h"
#include "ImportExternalDialog.h"
#include "ImportOtherUserDialog.h"
#include "ImportLocalDialog.h"
#include "PDebug.h"
#include <QProgressDialog>
#include "Tags.h"
#include "Extensions.h"
#include "SessionDB.h"
#include <QFileInfo>

ImportGUI::ImportGUI(class SessionDB *db,
                     class Scanner *scanner,
                     QList<QUrl> const &sources,
                     QObject *parent): QObject(parent) {
  extDlg = 0;
  otherUserDlg = 0;
  localDlg = 0;
  progressDlg = 0;
  
  job = new ImportJob(db, scanner, sources, this);

  connect(job, SIGNAL(complete(QString)),
          SLOT(finishUpCompletedJob(QString)));
  connect(job, SIGNAL(canceled()),
          SLOT(cleanUpCanceledJob()));
}

ImportGUI::~ImportGUI() {
  delete extDlg;
  delete otherUserDlg;
  delete localDlg;
  delete job;
}

void ImportGUI::showAndGo() {
  if (job->sourceInfo().isEmpty())
    cleanUpCanceledJob();
  else if (job->sourceInfo().isOtherUser()) 
    showAndGoOtherUser();
  else if (job->sourceInfo().isExternalMedia()
      || job->sourceInfo().isTemporaryLike()) 
    showAndGoExternal();
  else if (job->sourceInfo().isOnlyFolders()) 
    showAndGoLocal();
  else 
    showAndGoAltLocal();
}

void ImportGUI::showAndGoAltLocal() {
  showAndGoOtherUser();
  // This is not perfect, obviously, because Delete should be an option
  // for source disposition
}

void ImportGUI::showAndGoOtherUser() {
  job->setOperation(ImportJob::Operation::Import);
  job->setAutoCollection();
  otherUserDlg = new ImportOtherUserDialog(job,
					 Tags(job->database()).collections());
  job->countSources();
  connect(otherUserDlg, SIGNAL(accepted()), SLOT(dlgAcceptOtherUser()));
  connect(otherUserDlg, SIGNAL(canceled()), SLOT(dlgCancel()));
  otherUserDlg->show();
}

void ImportGUI::showAndGoLocal() {
  job->setOperation(ImportJob::Operation::Incorporate);
  job->setAutoCollection();
  localDlg = new ImportLocalDialog(job,
				   Tags(job->database()).collections());

  connect(localDlg, SIGNAL(accepted()), SLOT(dlgAcceptLocal()));
  connect(localDlg, SIGNAL(canceled()), SLOT(dlgCancel()));
  localDlg->show();
}

void ImportGUI::showAndGoExternal() {
  job->setOperation(ImportJob::Operation::Import);
  job->setAutoCollection();
  extDlg = new ImportExternalDialog(job, Tags(job->database()).collections());
  job->countSources();
  connect(extDlg, SIGNAL(accepted()), SLOT(dlgAcceptExternal()));
  connect(extDlg, SIGNAL(canceled()), SLOT(dlgCancel()));
  extDlg->show();
}  

void ImportGUI::finishUpCompletedJob(QString errmsg) {
  qDebug() << "finishUpCompletedJob" << errmsg;
  deleteLater();
}

void ImportGUI::cleanUpCanceledJob() {
  qDebug() << "cleanUpCanceledJob";
  deleteLater();
}

void ImportGUI::dlgAcceptOtherUser() {
  ASSERT(otherUserDlg);
  job->setCollection(otherUserDlg->collection());
  job->setDestination(otherUserDlg->destination());
  job->setSourceDisposition(CopyIn::SourceDisposition::Leave);
  if (otherUserDlg->hasMovieDestination())
    job->setMovieDestination(otherUserDlg->movieDestination());
  else
    job->setNoMovieDestination();
  if (otherUserDlg->incorporateInstead())
    job->setOperation(ImportJob::Operation::Incorporate);
  
  otherUserDlg->hide();

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

void ImportGUI::dlgAcceptLocal() {
  ASSERT(localDlg);
  job->setCollection(localDlg->collection());
  job->setDestination(localDlg->destination());
  job->setSourceDisposition(CopyIn::SourceDisposition::Leave);
  job->setNoMovieDestination();
  if (localDlg->importInstead())
    job->setOperation(ImportJob::Operation::Import);
  
  localDlg->hide();

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

void ImportGUI::dlgAcceptExternal() {
  ASSERT(extDlg);
  job->setCollection(extDlg->collection());
  job->setDestination(extDlg->destination());
  job->setSourceDisposition(extDlg->sourceDisposition());
  if (extDlg->hasMovieDestination())
    job->setMovieDestination(extDlg->movieDestination());
  else
    job->setNoMovieDestination();
  extDlg->hide();

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

bool ImportGUI::acceptable(QList<QUrl> const &urls) {
  for (auto const &url: urls) {
    if (!url.isLocalFile())
        return false;
    QFileInfo fi(url.path());
    if (fi.isDir())
      continue;
    else if (Extensions::imageExtensions().contains(fi.suffix().toLower()))
      continue;
    else
      return false;
  }
  return true;
}
