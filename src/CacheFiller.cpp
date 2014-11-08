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

void CacheFiller::recacheOne(quint64 version) {
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
  QMutexLocker l(mutex);
  n = 0;
  N = queueLength();
  try {
    while (!stopsoon) {
      qDebug() << "CacheFiller: running";
      QSet<quint64> ids;
      if (!(ids=checkQueue()).isEmpty()) {
	l.unlock();
	processSome(ids);
	qDebug() << "Progress: " << n << " / " << N;
	emit progressed(n, N);
	l.relock();
      } else {
	if (N>0)
	  emit done();
	n = 0;
	N = 0;
	waiter.wait(&mutex);
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
    

QSet<quint64> Scanner::checkQueue() {
  QSqlQuery qq(*db);
  qq.prepare("select version from queue limit 100");
  if (!qq.exec())
    throw qq;
  QSet<quint64> ids;
  while (qq.next()) 
    ids << qq.value(0).toULongLong();
  return ids;
}

void Scanner::processSome(QSet<quint64> versions) {
  