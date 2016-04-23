// Collector.cpp

#include "Collector.h"
#include "PDebug.h"
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include "Extensions.h"

Collector::Collector(QObject *parent): QThread(parent) {
  complete_ = false;
  cnt = 0;
}

Collector::~Collector() {
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
    if (url.isLocalFile()) {
      
      QFileInfo fi(url.path());
      if (fi.isDir())
        sourceDirs << url.path();
      else if (Extensions::imageExtensions().contains(fi.suffix().toLower()))
        imgFiles << fi.absoluteFilePath();
      else if (Extensions::movieExtensions().contains(fi.suffix().toLower()))
        movFiles << fi.absoluteFilePath();
      else
        qDebug() << "Collector: Ignoring unknown filetype" << fi.suffix();
    } else {
      qDebug() << "Collector: Ignoring non-local" << url.toString();
    }
  }
  cnt = imgFiles.size() + movFiles.size();
  emit progress(cnt, movFiles.size());

  while (!sourceDirs.isEmpty()) {
    if (cancel_) {
      emit canceled();
      return;
    }
    QDir dir(sourceDirs.takeFirst());
    for (QFileInfo const &fi: dir.entryInfoList(QDir::Dirs | QDir::Files
                                                | QDir::NoDotAndDotDot)) {
      if (fi.isDir())
        sourceDirs << fi.absoluteFilePath();
      else if (Extensions::imageExtensions().contains(fi.suffix().toLower()))
        imgFiles << fi.absoluteFilePath();
      else if (Extensions::movieExtensions().contains(fi.suffix().toLower()))
        movFiles << fi.absoluteFilePath();
    }
    cnt = imgFiles.size() + movFiles.size();
    emit progress(cnt, movFiles.size());
  }

  complete_ = true;
  emit complete();
}

int Collector::preliminaryCount() const {
  return cnt;
}
