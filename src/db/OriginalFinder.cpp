// OriginalFinder.cpp

#include "OriginalFinder.h"
#include "InterruptableFileReader.h"
#include "InterruptableRawReader.h"
#include "NoResult.h"
#include "Exif.h"

OriginalFinder::OriginalFinder(PhotoDB const &db, class BasicCache *cache,
                               QObject *parent):
  QObject(parent), db(db), cache(cache) {
  filereader = new InterruptableFileReader(this);
  rawreader = new InterruptableRawReader(this);
  connect(filereader, SIGNAL(ready(QString)),
          SLOT(freaderReady(QString)));
  connect(rawreader, SIGNAL(ready(QString)),
          SLOT(rreaderReady(QString)));
}

OriginalFinder::~OriginalFinder() {
}

void OriginalFinder::requestOriginal(quint64 version) {
  requestScaledOriginal(version, QSize(0, 0));
}

void OriginalFinder::requestScaledOriginal(quint64 vsn, QSize ds) {
  try {
    QSqlQuery q = db.query("select photo, mods from versions"
			   " where id=:a limit 1", vsn);
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
    osize = QSize(wid, hei);
    orient = Exif::Orientation(q.value(5).toInt());
    QString path = db.folder(folder) + "/" + fn;
    QString ext = db.ftype(ftype);
    InterruptableReader *reader = 0;
    if (ext=="nef" || ext=="cr2")
      reader = rawreader;
    else if (ext=="jpeg" || ext=="png" || ext=="tiff")
      reader = filereader;
    if (!reader)
      emit exception("OriginalFinder: Unknown file type");
    rawreader->cancel();
    filereader->cancel();
    desired = ds;
    version = vsn;
    filepath = path;
    reader->request(path);
  } catch (QSqlQuery &q) {
    emit exception("OriginalFinder: SqlError: " + q.lastError().text()
		   + " from " + q.lastQuery());
  } catch (...) {
    emit exception("OriginalFinder: Unknown exception");
  }  
}

void OriginalFinder::freaderReady(QString fn) {
  if (fn==filepath)
    read(filereader);
  else
    filereader->cancel(fn);
}

void OriginalFinder::rreaderReady(QString fn) {
  if (fn==filepath)
    read(rawreader);
  else
    filereader->cancel(fn);
}

void OriginalFinder::fixOrientation(Image16 &img) {
  switch (orient) {
  case Exif::Upright:
    break;
  case Exif::UpsideDown:
    img.rotate180();
    break;
  case Exif::CW:
    img.rotate90CW();
    break;
  case Exif::CCW:
    img.rotate90CCW();
    break;
  }
}  

void OriginalFinder::read(InterruptableReader *reader) {
  QByteArray data = reader->readAll(filepath);
  Image16 img = QImage::fromData(data);
  if (desired.isNull()) {
    fixOrientation(img); // this should happen later
    emit originalAvailable(version, img);
  } else {
    QSize desi = desired;
    if (orient==Exif::CW || orient==Exif::CCW) 
      desi = QSize(desired.height(), desired.width());
    img = img.scaled(desi, Qt::KeepAspectRatio);
    fixOrientation(img);
    emit scaledOriginalAvailable(version, osize, img);
  }
}
