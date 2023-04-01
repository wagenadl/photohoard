// ImportGUI.cpp

#include "ImportGUI.h"
#include "ImportJob.h"
#include "ImportExternalDialog.h"
#include "ImportOtherUserDialog.h"
#include "PDebug.h"
#include <QProgressDialog>
#include "Tags.h"
#include "Extensions.h"
#include "SessionDB.h"
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QPushButton>
#include <QFileDialog>
#include "ImportSelectorDialog.h"

ImportGUI::ImportGUI(class SessionDB *db,
                     class Scanner *scanner,
                     QList<QUrl> const &sources,
                     QObject *parent): QObject(parent) {
  extDlg = 0;
  otherUserDlg = 0;
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
  else 
    showAndGoAltLocal();
}

void ImportGUI::showAndGoAltLocal() {
  showAndGoExternal();
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
  if (progressDlg)
    progressDlg->deleteLater();
  deleteLater();
}

void ImportGUI::cleanUpCanceledJob() {
  qDebug() << "cleanUpCanceledJob";
  if (progressDlg)
    progressDlg->deleteLater();
  deleteLater();
}

void ImportGUI::dlgAcceptOtherUser() {
  ASSERT(otherUserDlg);

  QString dest = otherUserDlg->destination();
  bool contained = false;
  for (QString root: job->database()->rootFolders())
    if (dest.startsWith(root + "/"))
      contained = true;
  if (!contained) {
    QStringList roots = job->database()->rootFolders();
    QString tgt = (roots.size()==1)
      ? "the following folder"
      : "one of the following folders";
    QMessageBox::warning(otherUserDlg, "Photohoard",
                         "The destination must be a location that is"
                         " already included the database. In other words,"
                         " it must be (a subfolder of) " + tgt + ":\n\n  "
                         + roots.join("  \n"));
    return;
  }

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

void ImportGUI::dlgAcceptExternal() {
  ASSERT(extDlg);
  QString dest = extDlg->destination();
  bool contained = false;
  for (QString root: job->database()->rootFolders())
    if (dest.startsWith(root + "/"))
      contained = true;
  if (!contained) {
    QStringList roots = job->database()->rootFolders();
    QString tgt = (roots.size()==1)
      ? "the following folder"
      : "one of the following folders";
    QMessageBox::warning(extDlg, "Photohoard",
                         "The destination must be a location that is"
                         " already included the database. In other words,"
                         " it must be (a subfolder of) " + tgt + ":\n\n  "
                         + roots.join("  \n"));
    return;
  }
  
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

void ImportGUI::clickImportButton(SessionDB *db, Scanner *scanner,
                                  QWidget *parent) {
  pDebug() << "importbutton";
  QDir media("/media/" + qgetenv("USER"));
  if (!media.exists()) {
    qDebug() << "No media folder";
    return;
  }
  QStringList disks = media.entryList(QDir::Dirs | QDir::NoDotAndDotDot,
                                      QDir::Name);
  QStringList usedisks;
  for (auto d: disks) {
    qDebug() << "Disk: " << d;
    QDir dcim(media.absoluteFilePath(d + "/DCIM"));
    if (dcim.exists())
      usedisks << d;
  }

  QList<QUrl> urls;
  
  if (usedisks.isEmpty()) {
    QMessageBox box(QMessageBox::Question, "Photohoard: No media found",
                    "No removable media found."
                    " You may have to convince your operating system"
                    " to mount your media before trying again."
                    " Alternatively, click “Choose location” to manually"
                    " specify a source for import.",
                    QMessageBox::StandardButton(),
                    parent);
    auto choose = box.addButton("Choose location", QMessageBox::ActionRole);
    box.addButton(QMessageBox::Close);
    box.exec();
    if (box.clickedButton()==choose) {
      QString dir
        = QFileDialog::getExistingDirectory(parent,
                                            "Select source for import");
      if (dir.size()) 
        urls << QUrl::fromLocalFile(dir);
    }
  } else {
    QString choice = usedisks[0];
    if (usedisks.size() > 1) 
      choice = ImportSelectorDialog::choose(usedisks);
    pDebug() << "choice is " << choice;
    QString path = media.absoluteFilePath(choice + "/DCIM");
    urls << QUrl::fromLocalFile(path);
  }

  if (urls.size()) {
    qDebug() << urls;
    auto *gui = new ImportGUI(db, scanner, urls);
    gui->showAndGo();
  }
}


    
  
