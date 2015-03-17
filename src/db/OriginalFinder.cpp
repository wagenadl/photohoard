// OriginalFinder.cpp

#include "OriginalFinder.h"
#include "InterruptableFileReader.h"
#include "InterruptableRawReader.h"
#include "NoResult.h"
#include "Exif.h"
#include "PDebug.h"

OriginalFinder::OriginalFinder(PhotoDB const &db, 
                               QObject *parent):
  QObject(parent), db(db) {
  filereader = new InterruptableFileReader(this);
  rawreader = new InterruptableRawReader(this);
  connect(filereader, SIGNAL(ready(QString)),
          SLOT(provide(QString)));
  connect(rawreader, SIGNAL(ready(QString)),
          SLOT(provide(QString)));
}

OriginalFinder::~OriginalFinder() {
}

void OriginalFinder::requestOriginal(quint64 version) {
  requestScaledOriginal(version, PSize(0, 0));
}

PSize OriginalFinder::originalSize(quint64 vsn) {
  try {
    quint64 photo = db.simpleQuery("select photo from versions"
				   " where id=:a limit 1", vsn).toULongLong();
    QSqlQuery q = db.query("select width, height, orient "
               " from photos where id=:a limit 1", photo);
    if (!q.next())
      throw NoResult(__FILE__, __LINE__);
    PSize s(q.value(0).toInt(), q.value(1).toInt());
    return Exif::fixOrientation(s, Exif::Orientation(q.value(2).toInt()));
  } catch (...) {
    pDebug() << "OriginalFinder::originalSize: exception";
    return PSize();
  }
  return PSize(); // not executed
}		     

void OriginalFinder::requestScaledOriginal(quint64 vsn, QSize ds) {
  pDebug() << "requestScaledOriginal " << vsn << ds;
  try {
    quint64 photo = db.simpleQuery("select photo from versions"
				   " where id=:a limit 1", vsn).toULongLong();
    QSqlQuery q
      = db.query("select folder, filename, filetype, width, height, orient "
		 " from photos where id=:a limit 1", photo);
    if (!q.next())
      throw NoResult(__FILE__, __LINE__);
    quint64 folder = q.value(0).toULongLong();
    QString fn = q.value(1).toString();
    int ftype = q.value(2).toInt();
    int wid = q.value(3).toInt();
    int hei = q.value(4).toInt();
    orient = Exif::Orientation(q.value(5).toInt());
    osize = Exif::fixOrientation(PSize(wid, hei), orient);
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
    reader->request(path, Exif::fixOrientation(desired, orient),
                    Exif::fixOrientation(osize, orient)); // the reader doesn't
    // know about orientation, so we need to request in file shape
  } catch (QSqlQuery &q) {
    emit exception("OriginalFinder: SqlError: " + q.lastError().text()
		   + " from " + q.lastQuery());
  } catch (...) {
    emit exception("OriginalFinder: Unknown exception");
  }  
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

void OriginalFinder::provide(QString fn) {
  if (fn!=filepath)
    return;
  InterruptableReader::Result res = filereader->result(fn);
  QString e0 = res.error;
  if (!res.ok)
    res = rawreader->result(fn);
  if (!res.ok) {
    pDebug() << "  OF: got no result" << e0 << res.error;
    return;
  }

  Image16 img = QImage::fromData(res.data);
  if (img.isNull()) {
    pDebug() << "  got null image";
    return;
  }

  if (desired.isNull()) {
    fixOrientation(img);
    emit originalAvailable(version, img);
  } else {
    fixOrientation(img);
    emit scaledOriginalAvailable(version, osize, img);
  }
}
