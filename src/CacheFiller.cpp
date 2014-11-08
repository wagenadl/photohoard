// CacheFiller.cpp

#include "CacheFiller.h"
#include <QDebug>

class NoResult {
};

CacheFiller::CacheFiller(PhotoDB const &db, BasicCache *cache):
  db(db), cache(cache) {
  setObjectName("CacheFiller");
}

CacheFiller::~CacheFiller() {
}
  if (isRunning())
    stop();
  if (!wait(1000)) 
    qDebug() << "Failed to stop CacheFiller";
}

void CacheFiller::recache(QSet<quint64> versions) {
  QMutexLocker l(&mutex);
  Transaction t(cache->database());
  for (auto v: versions)
    recacheDirectly(v);
  t.commit();
  N += versions.size();
  waiter.wakeOne();
}

void CacheFiller::recacheDirectly(quint64 version) {
  QSqlQuery q(*cache->database());
  q.prepare("update cache set outdated=1 where version==:i");
  q.bindValue(":i", version);
  if (!q.exec())
    throw q;
  q.prepare("insert into queue values(:i)");
  q.bindValue(":i", version);
  if (!q.exec())
    throw q;
}

void CacheFiller::run() {
  mutex.lock();
  n = 0;
  N = queueLength();
  mutex.unlock();
  try {
    while (!stopsoon) {
      qDebug() << "CacheFiller: running";
      if (findVersionsToCache()) {
	qDebug() << "Progress: " << n << " / " << N;
	emit progressed(n, N);
      } else {
	if (N>0)
	  emit done();
	mutex.lock();
	n = 0;
	N = 0;
	waiter.wait(&mutex);
	mutex.unlock();
      }
    }
  } catch (QSqlQuery &q) {
    qDebug() << "CacheFiller: SqlError: " << q.lastError().text();
    qDebug() << "  from " << q.lastQuery();
    QMap<QString,QVariant> vv = q.boundValues();
    for (auto it=vv.begin(); it!=vv.end(); ++it) 
      qDebug() << "    " << it.key() << ": " << it.value();
    qDebug() << "  Thread terminating";
  } catch (std::system_error &e) {
    qDebug() << "Scanner: System error: "
	     << e.code().value() << e.code().message().c_str();
    qDebug() << "  Thread terminating";
  } catch (NoResult) {
    qDebug() << "Scanner: Expected object not found in table.";
    qDebug() << "  Thread terminating";
  } catch (...) {
    qDebug() << "Scanner: Unknown exception";
    qDebug() << "  Thread terminating";
  }
}
    
}
