// CacheFiller.cpp

#include "CacheFiller.h"
#include <QDebug>
#include "Exif.h"
#include <system_error>
#include <QTime>

class NoResult {
};

CacheFiller::CacheFiller(PhotoDB const &db, BasicCache *cache):
  db(db), cache(cache) {
  setObjectName("CacheFiller");
  QSqlQuery q(*db);
  q.prepare("select id, stdext from filetypes");
  if (!q.exec()) {
    qDebug() << "Could not select extensions";
    throw q.lastError();
  }
  while (q.next()) 
    ftypes[q.value(0).toInt()] = q.value(1).toString();
}

CacheFiller::~CacheFiller() {
}

void CacheFiller::recache(QSet<quint64> versions) {
  QMutexLocker l(&mutex);
  qDebug() << "recache";
  Transaction t(cache->database());
  for (auto v: versions)
    recacheOne(v);
  t.commit();
  qDebug() << "recache committed";
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
  try {
    QMutexLocker l(&mutex);
    n = 0;
    N = queueLength();
    while (!stopsoon) {
      QTime t; t.start();
      QSet<quint64> ids;
      if (!(ids=checkQueue()).isEmpty()) {
	l.unlock();
	processSome(ids);
	qDebug() << "Cache Progress: " << n << " / " << N;
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
    
int CacheFiller::queueLength() {
  QSqlQuery q(*cache->database());
  q.prepare("select count(*) from queue");
  if (!q.exec())
    throw q;
  if (!q.next())
    throw NoResult();
  return q.value(0).toInt();
}

QSet<quint64> CacheFiller::checkQueue() {
  QSqlQuery qq(*cache->database());
  qq.prepare("select version from queue limit 100");
  if (!qq.exec())
    throw qq;
  QSet<quint64> ids;
  while (qq.next()) 
    ids << qq.value(0).toULongLong();
  return ids;
}

void CacheFiller::processSome(QSet<quint64> versions) {
  QTime t0; t0.start();
  QSqlQuery q(*db);
  QMap<quint64, QString> folders;

  QMap<quint64, QImage> images;
  for (auto v: versions) {
    q.prepare("select photo, ver from versions where id=:v");
    q.bindValue(":v", v);
    if (!q.exec())
      throw q;
    if (!q.next())
      throw NoResult();
    quint64 photo = q.value(0).toULongLong();
    int ver = q.value(1).toInt();
    q.prepare("select folder, filename, filetype, orient "
              " from photos where id=:i");
    q.bindValue(":i", photo);
    if (!q.exec())
      throw q;
    if (!q.next())
      throw NoResult();
    quint64 folder = q.value(0).toULongLong();
    QString fn = q.value(1).toString();
    int ftype = q.value(2).toInt();
    //    int wid = q.value(3).toInt();
    //    int hei = q.value(4).toInt();
    Exif::Orientation orient = Exif::Orientation(q.value(5).toInt());
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

    
    QImage img = getImage(path, ver, ftype, orient);
    images[v] = cache->sufficientSize(img);
  }

  Transaction t(cache->database());
  for (auto v: versions) {
    // Perhaps I could do better by passing size as well for some file types?
    
    if (images[v].isNull()) {
      qDebug() << "Null image from getImage for " << v;
      cache->remove(v);
    } else {
      cache->add(v, images[v]);
    }
  }
  QSqlQuery qq(*cache->database());
  for (auto v: versions) {
    qq.prepare("delete from queue where version==:i");
    qq.bindValue(":i", v);
    if (!qq.exec())
      throw qq;
  }
  t.commit();

  n += versions.size();
}

void CacheFiller::upsideDown(QImage &img) {
  int N = img.bytesPerLine()*img.height();
  if (img.format()==QImage::Format_Indexed8) {
    unsigned char *bits = img.bits();
    unsigned char *end = bits + N;
    end -= img.bytesPerLine()-img.width();
    int n = N/2;
    while (--n>=0) {
      uchar b = *bits;
      *bits++ = *--end;
      *end = b;
    }
  } else {
    quint64 *bits = (quint64*)img.bits();
    quint64 *end = bits + N/4;
    end -= img.bytesPerLine() - 4*img.width();
    int n = N/8;
    while (--n>=0) {
      quint64 b = *bits;
      *bits++ = *--end;
      *end = b;
    }
  }
}

QImage CacheFiller::rotateCW(QImage const &img) {
  int W = img.height();
  int H = img.width();
  QImage im2(QSize(W, H), img.format());
  if (img.format()==QImage::Format_Indexed8) {
    unsigned char const *src = img.bits();
    unsigned char *dst = im2.bits();
    for (int y=0; y<H; y++) 
      for (int x=0; x<W; x++) 
        *dst++ = src[(H-1-y) + H*x];
  } else {
    quint64 const *src = (quint64 const *)img.bits();
    quint64 *dst = (quint64 *)im2.bits();
    for (int y=0; y<H; y++) 
      for (int x=0; x<W; x++) 
        *dst++ = src[(H-1-y) + H*x];
  }
  return im2;
}

QImage CacheFiller::rotateCCW(QImage const &img) {
  int W = img.height();
  int H = img.width();
  QImage im2(QSize(W, H), img.format());
  if (img.format()==QImage::Format_Indexed8) {
    unsigned char const *src = img.bits();
    unsigned char *dst = im2.bits();
    for (int y=0; y<H; y++) 
      for (int x=0; x<W; x++) 
        *dst++ = src[y + H*(W-1-x)];
  } else {
    quint64 const *src = (quint64 const *)img.bits();
    quint64 *dst = (quint64 *)im2.bits();
    for (int y=0; y<H; y++) 
      for (int x=0; x<W; x++) 
        *dst++ = src[y + H*(W-1-x)];
  }
  return im2;
}

QImage CacheFiller::getImage(QString path, int ver, int ftype,
                             Exif::Orientation orient) {
  qDebug() << "Caching " << path << ver << ftype << (int)orient;
  if (ver!=0)
    return QImage();

  QString ext = ftypes[ftype];
  if (ext=="jpeg" || ext=="png" || ext=="tiff") {
    // Can do
    QImage img(path);
    if (img.isNull())
      return img;
    
  } else if (ext=="nef" || ext=="cr2") {
    // Need dcraw
    return QImage();
  } else {
    // Unknown format
    return QImage();
  }
  return QImage(); // Cannot be reached, I believe.
}
