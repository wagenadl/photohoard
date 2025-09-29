// Collector.cpp

#include "Collector.h"
#include "PDebug.h"
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include "Extensions.h"

Collector::Collector(QObject *parent): QThread(parent) {
  pDebug() << "Collector" << this;
  complete_ = false;
  cnt = 0;
  movcnt = 0;
}

Collector::~Collector() {
  pDebug() << "~Collector" << this;
  if (isRunning()) {
    cancel();
    qDebug() << "Collector: Destructing while active. Canceling, and waiting.";
    if (!wait(10000)) {
      qDebug() << "Failed to stop Collector thread. Aborting.";
      ASSERT(0);
    }
  }
}

bool Collector::isComplete() const {
  return complete_;
}

QStringList const &Collector::imageFiles() const {
  ASSERT(complete_);
  return imgFiles;
}

QStringList const &Collector::movieFiles() const {
  ASSERT(complete_);
  return movFiles;
}

void Collector::collect(QList<QUrl> urls0) {
  ASSERT(!isRunning());
  urls = urls0;
  cancel_ = false;
  complete_ = false;
  cnt = 0;
  imgFiles.clear();
  movFiles.clear();
  start();
}

void Collector::cancel() {
  cancel_ = true;
}

void Collector::run() {
  QList<QString> sourceDirs;
  for (QUrl const &url: urls) {
    if (cancel_) {
      emit canceled();
      return;
    }
    QSet<QString> const &imgext = Extensions::imageExtensions();
    QSet<QString> const &movext = Extensions::imageExtensions();
    if (url.isLocalFile()) {
      QFileInfo fi(url.path());
      qDebug() << "Collector: " << fi.absoluteFilePath();
      QString sfx = fi.suffix().toLower();
      if (fi.isDir()) {
        sourceDirs << url.path();
      } else if (imgext.contains(sfx)) {
        imgFiles << fi.absoluteFilePath();
        updateDates(fi.lastModified());
      } else if (movext.contains(sfx)) {
        movFiles << fi.absoluteFilePath();
      } else {
        qDebug() << "Collector: Ignoring unknown filetype" << fi.suffix();
      }
    } else {
      qDebug() << "Collector: Ignoring non-local" << url.toString();
    }
  }
  cnt = imgFiles.size() + movFiles.size();
  complete_ = sourceDirs.isEmpty();
  emit progress(cnt, movFiles.size(), complete_);

  while (!sourceDirs.isEmpty()) {
    if (cancel_) {
      emit canceled();
      return;
    }
    QDir dir(sourceDirs.takeFirst());
    for (QFileInfo const &fi: dir.entryInfoList(QDir::Dirs | QDir::Files
                                                | QDir::NoDotAndDotDot)) {
      QString sfx = fi.suffix().toLower();
      if (fi.isDir()) {
        sourceDirs << fi.absoluteFilePath();
      } else if (Extensions::imageExtensions().contains(sfx)) {
        imgFiles << fi.absoluteFilePath();
        updateDates(fi.lastModified());
      } else if (Extensions::movieExtensions().contains(sfx)) {
        movFiles << fi.absoluteFilePath();
      }
    }
    movcnt = movFiles.size();
    cnt = imgFiles.size() + movcnt;
    complete_ = sourceDirs.isEmpty();
    emit progress(cnt, movcnt, complete_);
  }

  qDebug() << "collector complete";
  emit complete();
}

int Collector::preliminaryCount() const {
  return cnt;
}

int Collector::preliminaryMovieCount() const {
  return movcnt;
}

void Collector::updateDates(QDateTime dt) {
  if (imgFiles.size() <= 1) 
    dt0 = dt1 = dt;
  else if (dt < dt0) 
    dt0 = dt;
  else if (dt > dt1)
    dt1 = dt;
}

QDateTime Collector::firstDate() const {
  return dt0;
}

QDateTime Collector::lastDate() const {
  return dt1;
}

