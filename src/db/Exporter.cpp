// Exporter.cpp

#include "Exporter.h"
#include <QMutexLocker>
#include <QDebug>
#include "IF_Worker.h"
#include "NoResult.h"
#include <QRegExp>
#include <QDir>

Exporter::Exporter(PhotoDB const &db, QObject *parent):
  QThread(parent), db(db) {
  qRegisterMetaType< QSet<quint64> >("QSet<quint64>");
  readFTypes();
  worker = new IF_Worker(this);
}

Exporter::~Exporter() {
  if (isRunning()) {
    qDebug() << "Warning: Exporter destroyed while running";
    stopsoon = true;
    cond.wakeOne();
    if (wait(1000)) {
      qDebug() << "Exporter stopped. All is well.";
    } else {
      terminate();
      if (wait(1000)) {
        qDebug() << "Exporter terminated.";
      } else {
        qDebug() << "Could not terminate exporter";
      }
    }
  }
}

void Exporter::setup(ExportSettings const &s) {
  QMutexLocker l(&mutex);
  if (jobs.isEmpty() || !jobs.last().todo.isEmpty())
    jobs << Job();
  jobs.last().settings = s;
  settings = s;
}

void Exporter::addSelection() {
  QSet<quint64> vsns;
  QSqlQuery q(db.query("select version from selection"));
  while (q.next())
    vsns << q.value(0).toULongLong();
  if (!vsns.isEmpty())
    add(vsns);
}

void Exporter::add(QSet<quint64> const &vsns) {
  QMutexLocker l(&mutex);
  if (jobs.isEmpty()) {
    jobs << Job();
    jobs.last().settings = settings;
  }
  bool wasEmpty = jobs.last().todo.isEmpty();
  jobs.last().todo |= vsns;
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
  cond.wakeOne();
  if (!wait(10000))
    qDebug() << "Warning: Exporter: failed to stop";
}

void Exporter::run() {
  QTime t0;
  t0.start();
  mutex.lock();
  while (!stopsoon) {
    while (!jobs.isEmpty()) {
      Job &job(jobs.first());
      if (job.todo.isEmpty())
        break;

      quint64 vsn = *job.todo.begin();
      mutex.unlock();
      bool ok = doExport(vsn, job.settings);
      mutex.lock();

      if (ok)
        job.completed << vsn;
      else
        job.failed << vsn;
      job.todo.remove(vsn);
      
      if (t0.elapsed()>=1000 || job.todo.isEmpty()) {
        int nleft = job.todo.size();
        int ndone = job.completed.size() + job.failed.size();
        emit progress(ndone, nleft+ndone);
        t0.restart();
      }
      if (job.todo.isEmpty()) {
        emit completed(job.settings.destination,
                       job.completed.size(), job.failed.size());
        jobs.removeFirst();
      }
    }
    cond.wait(&mutex);
  }
  mutex.unlock();
}

bool Exporter::doExport(quint64 vsn, ExportSettings const &settings) {
  
  QSqlQuery q(db.query("select photo, mods from versions"
                       " where id=:a limit 1", vsn));
  if (!q.next())
    return false;
  quint64 photo = q.value(0).toULongLong();
  QString mods = q.value(1).toString();

  q = db.query("select folder, filename, filetype, width, height, orient, "
               " capturedate"
               " from photos where id=:a limit 1", photo);
  if (!q.next())
    return false;

  quint64 folder = q.value(0).toULongLong();
  QString fn = q.value(1).toString();
  int ftype = q.value(2).toInt();
  int wid = q.value(3).toInt();
  int hei = q.value(4).toInt();
  Exif::Orientation orient = Exif::Orientation(q.value(5).toInt());
  QDateTime date = q.value(6).toDateTime();
  
  if (!folders.contains(folder)) {
    q.prepare("select pathname from folders where id=:i");
    q.bindValue(":i", folder);
    if (!q.exec())
      throw q;
    if (!q.next())
      throw NoResult();
    folders[folder] = q.value(0).toString();
  }
  QString path = folders[folder] + "/" + fn;

  int maxdim = 0;
  switch (settings.resolutionMode) {
  case ExportSettings::ResolutionMode::Full:
    break;
  case ExportSettings::ResolutionMode::LimitWidth:
    if (wid>=hei)
      maxdim = settings.maxdim;
    else
      maxdim = settings.maxdim * wid/hei;
    break;
  case ExportSettings::ResolutionMode::LimitHeight:
    if (hei>=wid)
      maxdim = settings.maxdim;
    else
      maxdim = settings.maxdim * hei/wid;
    break;
  case ExportSettings::ResolutionMode::LimitMaxDim:
    maxdim = settings.maxdim;
    break;
  case ExportSettings::ResolutionMode::Scale:
    if (wid>=hei)
      maxdim = wid*settings.scalePercent/100;
    else
      maxdim = hei*settings.scalePercent/100;
    break;
  }

  QImage img = worker->findImageNow(path, mods, ftypes[ftype], orient,
                                    0 /*maxdim*/, QSize(wid, hei));

  if (img.isNull())
    return false;

  // Current scaling system is primitive. We should do bicubic.
  // And of course, we should support higher bit depths.
  switch (settings.resolutionMode) {
  case ExportSettings::ResolutionMode::Full:
    break;
  case ExportSettings::ResolutionMode::LimitWidth:
    if (img.width() > settings.maxdim)
      img = img.scaledToWidth(settings.maxdim, Qt::SmoothTransformation);
    break;
  case ExportSettings::ResolutionMode::LimitHeight:
    if (img.height() > settings.maxdim)
      img = img.scaledToHeight(settings.maxdim, Qt::SmoothTransformation);
    break;
  case ExportSettings::ResolutionMode::LimitMaxDim:
    if (wid>hei) {
      if (img.width() > settings.maxdim)
        img = img.scaledToWidth(settings.maxdim, Qt::SmoothTransformation);
    } else {
    if (img.height() > settings.maxdim)
      img = img.scaledToHeight(settings.maxdim, Qt::SmoothTransformation);
    }      
    break;
  case ExportSettings::ResolutionMode::Scale:
    if (img.width() > settings.scalePercent*wid/100)
      img = img.scaledToWidth(settings.scalePercent*wid/100,
                              Qt::SmoothTransformation);
    break;
  }

  QString ofn;
  switch (settings.namingScheme) {
  case ExportSettings::NamingScheme::Original:
    ofn = fn;
    if (ofn.contains("."))
      ofn = ofn.left(ofn.lastIndexOf("."));
    break;
  case ExportSettings::NamingScheme::DateTime:
    ofn = date.toString("yyMMdd-hhmmss");
    break;
  case ExportSettings::NamingScheme::DateTimeDSC: {
    ofn = date.toString("yyMMdd-hhmmss");
    QRegExp dd("(\\d+)");
    if (dd.indexIn(fn)>=0)
      ofn += "-" + dd.cap(1);
    else
      ofn += "_" + QString::number(vsn);
  } break;
  }

  QDir root(QDir::root());
  if (!root.exists(settings.destination)) 
    root.mkpath(settings.destination);
  
  ofn = settings.destination + "/" + ofn + "." + settings.extension();

  if (settings.fileFormat == ExportSettings::FileFormat::JPEG)
    return img.save(ofn, 0, settings.jpegQuality);
  else
    return img.save(ofn);
}

void Exporter::readFTypes() {
  QSqlQuery q(*db);
  q.prepare("select id, stdext from filetypes");
  if (!q.exec()) {
    qDebug() << "Could not select extensions";
    throw q;
  }
  while (q.next()) 
    ftypes[q.value(0).toInt()] = q.value(1).toString();
}