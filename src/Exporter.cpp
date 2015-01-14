// Exporter.cpp

#include "Exporter.h"
#include <QMutexLocker>
#include <QDebug>

Exporter::Exporter(PhotoDB const &db, QObject *parent):
  QThread(parent), db(db) {
  qRegisterMetaType< QSet<quint64> >("QSet<quint64>");
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
  qDebug() << "Export " << vsn << " to " << settings.destination;
  return false;
}

