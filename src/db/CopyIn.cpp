// CopyIn.cpp

#include "CopyIn.h"
#include "PDebug.h"
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QDate>
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

void CopyIn::setSources(QStringList s) {
  imgSources = s;
}

void CopyIn::setMovieSources(QStringList s) {
  movSources = s;
}

void CopyIn::setSourceDisposition(CopyIn::SourceDisposition d) {
  srcdisp = d;
}

bool CopyIn::isValid() const {
  if (dest.isEmpty() && !imgSources.isEmpty())
    return false;
  if (moviedest.isEmpty() && !movSources.isEmpty())
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
  if (!root.mkpath(dest) && !imgSources.isEmpty()) {
    emit completed(0, imgSources.size() + movSources.size());
    return;
  }

  if (!moviedest.isEmpty() && !movSources.isEmpty()) {
    if (!root.mkpath(moviedest)) {
      emit completed(0, imgSources.size() + movSources.size());
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
    
  int nok=0, nmov=0, nfail=0;
  int ntot = imgSources.size() + movSources.size();
  QString lbl = ntot==1 ? "file" : "files";
  QList<QString> disposableSources;
  
  for (QString s: imgSources) {
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
      qDebug() << "Copied" << s << "as" << f_out;
    } else {
      nfail ++;
      qDebug() << "Copy failed on " << s;
    }
    emit progress(nok + nmov + nfail);
    Messenger::message(this, QString("Copied %1/%2 %3")
                       .arg(nok + nmov).arg(ntot).arg(lbl));
  }
  
  for (QString s: movSources) {
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
    emit progress(nok + nmov + nfail);
    Messenger::message(this, QString("Copied %1/%2 %3")
                       .arg(nok + nmov).arg(ntot).arg(lbl));
  }

  if (!disposableSources.isEmpty() && srcdisp!=Leave) {
    COMPLAIN("CopyIn: Source disposition NYI");
  }
  Messenger::message(this, "Copying complete");
  emit completed(nok + nmov, nfail);
}

QString CopyIn::autoDest(QString path) {
  QString sub = "photohoard/"
    + QDate::currentDate().toString("yyyy/yyMMdd");
  if (path.isEmpty())
    return sub;
  else if (path.endsWith("/"))
    return path + sub;
  else
    return path + "/" + sub;
}
