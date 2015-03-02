// OriginalFinder.cpp

#include "OriginalFinder.h"
#include "IF_Worker.h"
#include "NoResult.h"

OriginalFinder::OriginalFinder(PhotoDB const &db, class BasicCache *cache, QObject *parent):
  QObject(parent), db(db), cache(cache) {
  ifinder = new IF_Worker(this);
  readFTypes();
}

OriginalFinder::~OriginalFinder() {
}

void OriginalFinder::readFTypes() {
  QSqlQuery q(*db);
  q.prepare("select id, stdext from filetypes");
  if (!q.exec()) {
    qDebug() << "Could not select extensions";
    throw q;
  }
  while (q.next()) 
    ftypes[q.value(0).toInt()] = q.value(1).toString();
}

void OriginalFinder::requestOriginal(quint64 version) {
  requestScaledOriginal(version, QSize(0, 0));
}

void OriginalFinder::requestScaledOriginal(quint64 version, QSize desired) {
  try {
    QSqlQuery q = db.query("select photo, mods from versions"
			   " where id=:a limit 1", version);
    if (!q.next())
      throw NoResult();
    quint64 photo = q.value(0).toULongLong();
    QString mods = q.value(1).toString();
    if (mods=="") {
      // We might have it in our cache.
      // But for right now, I am not bothering to look.
    }
    q = db.query("select folder, filename, filetype, width, height, orient "
               " from photos where id=:a limit 1", photo);
    if (!q.next())
      throw NoResult();
    quint64 folder = q.value(0).toULongLong();
    QString fn = q.value(1).toString();
    int ftype = q.value(2).toInt();
    int wid = q.value(3).toInt();
    int hei = q.value(4).toInt();
    Exif::Orientation orient = Exif::Orientation(q.value(5).toInt());
    if (!folders.contains(folder)) 
      folders[folder]
	= db.simpleQuery("select pathname from folders where id=:i", folder)
	.toString();
    QString path = folders[folder] + "/" + fn;
    Image16 img = ifinder->findImageNow(path, "",
					ftypes[ftype], orient,
					desired.width()>desired.height()
					?desired.width():desired.height(),
					QSize(wid, hei));
    if (desired.width()>0)
      emit scaledOriginalAvailable(version, desired, img);
    else
      emit originalAvailable(version, img);
  } catch (QSqlQuery &q) {
    emit exception("OriginalFinder: SqlError: " + q.lastError().text()
		   + " from " + q.lastQuery());
  } catch (...) {
    emit exception("OriginalFinder: Unknown exception");
  }  
}

