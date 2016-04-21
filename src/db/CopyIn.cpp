// CopyIn.cpp

#include "CopyIn.h"
#include "PDebug.h"
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include "Messenger.h"
#include "Extensions.h"

CopyIn::CopyIn(QObject *parent): QThread(parent) {
  srcdisp = Leave;
}

CopyIn::~CopyIn() {
  if (isRunning()) {
    cancel();
    qDebug() << "CopyIn: Destructing while active. Canceling, and waiting.";
    if (!wait(10000)) {
      qDebug() << "Failed to stop CopyIn thread. Aborting.";
      ASSERT(0);
    }
  }
}

void CopyIn::setDestination(QString d) {
  dest = d;
}

void CopyIn::setMovieDestination(QString d) {
  moviedest = d;
}

void CopyIn::setNoMovieDestination() {
  moviedest = "";
}

void CopyIn::setSources(QList<QUrl> urls) {
  src = urls;
}

void CopyIn::setSourceDisposition(CopyIn::SourceDisposition d) {
  srcdisp = d;
}

bool CopyIn::isValid() const {
  if (dest.isEmpty())
    return false;
  for (QUrl const &url: src) 
    if (!url.isLocalFile())
      return false;
  return true;
}

void CopyIn::start() {
  ASSERT(isValid());
  ASSERT(!isRunning());
  cancel_ = false;
  QThread::start();
}

void CopyIn::cancel() {
  cancel_ = true;
}

void CopyIn::run() {
  // First, let's make sure destination exists
  QDir root("/");
  if (!root.mkpath(dest)) {
    emit completed(0, src.size());
    return;
  }

  if (!moviedest.isEmpty()) {
    if (!root.mkpath(moviedest)) {
      emit completed(0, src.size());
      return;
    }
  }
      
  QDir dst(dest);
  QDir mdst(moviedest);
  
  QList<QString> copiedFiles;

  auto doCancel = [this,copiedFiles]() {
    qDebug() << "CopyIn: Canceling...";
    for (QString const &s: copiedFiles) {
      QFile(s).remove();
    }
    emit canceled();
  };    
    
  QList<QString> sourceFiles;
  QList<QString> movieFiles;
  QList<QString> sourceDirs;
  for (QUrl const &url: src) {
    if (cancel_) {
      doCancel();
      return;
    }
    if (url.isLocalFile()) {
      QFileInfo fi(url.path());
      if (fi.isDir())
        sourceDirs << url.path();
      else
        sourceFiles << url.path();
    } else {
      qDebug() << "Ignoring non-local" << url.toString();
    }
  }

  while (!sourceDirs.isEmpty()) {
    if (cancel_) {
      doCancel();
      return;
    }
    QDir dir(sourceDirs.takeFirst());
    for (QFileInfo const &fi: dir.entryInfoList(QDir::Dirs | QDir::Files
                                                | QDir::NoDotAndDotDot)) {
      if (fi.isDir())
        sourceDirs << fi.absoluteFilePath();
      else if (Extensions::imageExtensions().contains(fi.suffix().toLower()))
        sourceFiles << fi.absoluteFilePath();
      else if (Extensions::movieExtensions().contains(fi.suffix().toLower())
               && !moviedest.isEmpty())
        movieFiles << fi.absoluteFilePath();
    }
  }

  int nok=0, nmov=0, nfail=0;
  int ntot = sourceFiles.size() + movieFiles.size();
  QString lbl = ntot==1 ? "file" : "files";
  QList<QString> disposableSources;
  
  for (QString s: sourceFiles) {
    if (cancel_) {
      doCancel();
      return;
    }
    QFile f_in(s);
    QFileInfo fi(f_in);
    QString f_out = dst.absoluteFilePath(fi.fileName().toLower());
    if (f_in.copy(f_out)) {
      copiedFiles << f_out;
      disposableSources << s;
      nok ++;
    } else {
      nfail ++;
    }
    Messenger::message(this, QString("Copied %1/%2 %3")
                       .arg(nok + nmov).arg(ntot).arg(lbl));
  }
  
  for (QString s: movieFiles) {
    if (cancel_) {
      doCancel();
      return;
    }
    QFile f_in(s);
    QFileInfo fi(f_in);
    QString f_out = mdst.absoluteFilePath(fi.fileName().toLower());
    if (f_in.copy(f_out)) {
      copiedFiles << f_out;
      disposableSources << s;
      nmov ++;
    } else {
      nfail ++;
    }
    Messenger::message(this, QString("Copied %1/%2 %3")
                       .arg(nok + nmov).arg(ntot).arg(lbl));
  }
        
  Messenger::message(this, "Copying complete");
  emit completed(nok + nmov, nfail);
}
