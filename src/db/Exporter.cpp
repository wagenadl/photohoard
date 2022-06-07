// Exporter.cpp

#include "Exporter.h"
#include <QProcess>
#include <QMutexLocker>
#include "PDebug.h"
#include "IF_Worker.h"
#include <QRegExp>
#include <QDir>
#include "Adjustments.h"
#include <QApplication>
#include <QClipboard>
#include <QElapsedTimer>

Exporter::Exporter(SessionDB *db0, QObject *parent):
  QThread(parent), db0(db0) {
  db.clone(*db0);
  qRegisterMetaType< QSet<quint64> >("QSet<quint64>");
  worker = new IF_Worker(this);
}

Exporter::~Exporter() {
  if (isRunning()) {
    COMPLAIN("Warning: Exporter destroyed while running");
    stopsoon = true;
    cond.wakeOne();
    if (!wait(1000)) {
      COMPLAIN("Warning: Exporter won't stop - terminating");
      terminate();
      if (!wait(1000)) {
        CRASH("Could not terminate exporter");
      }
    }
  }
  db.close();
}

void Exporter::setup(ExportSettings const &s) {
  QMutexLocker l(&mutex);
  if (jobs.isEmpty() || !jobs.last().todo.isEmpty())
    jobs << Job();
  jobs.last().settings = s;
  settings_ = s;
}

void Exporter::copyFilenameToClipboard(quint64 vsn) {
  /* To be called from parent thread only. */
  if (settings().isValid()) {
    QString fn = settings().exportFilename(db0, vsn);
    //pDebug() << "filename: " << fn;
    QApplication::clipboard()->setText(fn);
  } else {
    qDebug() << "Exporter settings not valid—nothing to copy";
  }
}


void Exporter::addSelection() {
  QSet<quint64> vsns;
  QSqlQuery q(db0->query("select version from selection"));
  while (q.next())
    vsns << q.value(0).toULongLong();
  q.finish();
  if (!vsns.isEmpty())
    add(vsns);
}

void Exporter::add(QSet<quint64> const &vsns) {
  if (vsns.size()>=1)
    copyFilenameToClipboard(*vsns.begin());
  QMutexLocker l(&mutex);
  if (jobs.isEmpty()) {
    jobs << Job();
    jobs.last().settings = settings_;
  }
  bool wasEmpty = jobs.last().todo.isEmpty();
  jobs.last().todo |= vsns;
  if (wasEmpty)
    cond.wakeOne();
}

void Exporter::add(quint64 vsn, QString ofn) {
  QApplication::clipboard()->setText(ofn);
  QMutexLocker l(&mutex);
  if (jobs.isEmpty()) {
    jobs << Job();
    jobs.last().settings = settings_;
  }
  bool wasEmpty = jobs.last().todo.isEmpty();
  jobs.last().todo << vsn;
  jobs.last().fnoverride[vsn] = ofn;
  if (wasEmpty)
    cond.wakeOne();
}

void Exporter::start() {
  stopsoon = false;
  if (!isRunning())
    QThread::start();
}

void Exporter::stop() {
  stopsoon = true;
  if (!isRunning())
    return;
  //pDebug() << "Exporter: stop";
  mutex.lock();
  //pDebug() << "Exporter: stop: lock";
  cond.wakeOne();
  mutex.unlock();
  //  pDebug() << "Sent wakeup";
  if (!wait(10000))
    COMPLAIN("Warning: Exporter: failed to stop");
}

void Exporter::run() {
  QElapsedTimer t0;
  t0.start();
  mutex.lock();
  //  pDebug() << "Exporter running";
  while (!stopsoon) {
    //    pDebug() << "Not yet stopping";
    while (!jobs.isEmpty()) {
      Job &job(jobs.first());
      if (job.todo.isEmpty()) {
        emit completed(job.settings.destination,
                       job.completed.size(), job.failed.size());
        jobs.removeFirst();
        break;
      }

      quint64 vsn = *job.todo.begin();
      QString ovr = job.fnoverride.contains(vsn)
        ? job.fnoverride[vsn] : QString();
      mutex.unlock();
      QString ofn = doExport(vsn, job.settings, ovr);
      bool ok = !ofn.isEmpty();
      mutex.lock();

      if (ok) {
        job.completed << vsn;
        emailready.insert(ofn);
      } else {
        job.failed << vsn;
      }
      job.todo.remove(vsn);
      
      if (t0.elapsed()>=1000 || job.todo.isEmpty()) {
        int nleft = job.todo.size();
        int ndone = job.completed.size() + job.failed.size();
        emit progress(ndone, nleft+ndone);
        t0.restart();
      }
    }
    if (!stopsoon) {
      //      pDebug() << "Exporter waiting";
      cond.wait(&mutex);
      //pDebug() << "Exporter wakeup";
    }
  }
  //  pDebug() << "Exporter out of loop";
  mutex.unlock();
  //  pDebug() << "Exporter end run";
}


QString Exporter::doExport(quint64 vsn, ExportSettings const &settings,
                           QString fnoverride) {
  PhotoDB::VersionRecord vrec = db.versionRecord(vsn);
  PhotoDB::PhotoRecord prec = db.photoRecord(vrec.photo);
  AllAdjustments adjs = AllAdjustments::fromDB(vsn, db);
  PSize cropsize = adjs.baseAdjustments().cropSize(prec.filesize, vrec.orient);
  
  QString path = db.folder(prec.folderid) + "/" + prec.filename;
  Image16 img = worker
    ->findImageNow(path, db.ftype(prec.filetype), vrec.orient, prec.filesize,
                   adjs, 0, true);
  if (img.isNull())
    return "";

  // Current scaling system is primitive. We should do bicubic.
  // And of course, we should support higher bit depths.
  switch (settings.resolutionMode) {
  case ExportSettings::ResolutionMode::Full:
    break;
  case ExportSettings::ResolutionMode::LimitWidth:
    if (img.width() > settings.maxdim)
      img = img.scaledToWidth(settings.maxdim);
    break;
  case ExportSettings::ResolutionMode::LimitHeight:
    if (img.height() > settings.maxdim)
      img = img.scaledToHeight(settings.maxdim);
    break;
  case ExportSettings::ResolutionMode::LimitMaxDim:
    if (img.width()>img.height()) {
      if (img.width() > settings.maxdim)
        img = img.scaledToWidth(settings.maxdim);
    } else {
      if (img.height() > settings.maxdim)
        img = img.scaledToHeight(settings.maxdim);
    }      
    break;
  case ExportSettings::ResolutionMode::Scale:
    if (img.width() > settings.scalePercent*cropsize.width()/100)
      img = img.scaledToWidth(settings.scalePercent*cropsize.width()/100);
    break;
  }

  QString ofn = fnoverride.isEmpty()
    ? settings.exportFilename(&db, vsn)
    : fnoverride;

  if (settings.fileFormat == ExportSettings::FileFormat::JPEG)
    return img.toQImage().save(ofn, 0, settings.jpegQuality) ? ofn : "";
  else
    return img.toQImage().save(ofn) ? ofn : "";
}

void Exporter::sendEmail() {
  mutex.lock();
  QSet<QString> fns = emailready;
  emailready.clear();
  mutex.unlock();
  if (fns.isEmpty()) {
    qDebug() << "Nothing to email";
    return;
  }

  QStringList args;
  for (QString fn: fns)
    args << "--attach" << fn;
  QProcess::startDetached("xdg-email", args);
}
