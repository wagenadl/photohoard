// OriginalFinder.cpp

#include "OriginalFinder.h"
#include "InterruptableFileReader.h"
#include "InterruptableRawReader.h"
#include "Exif.h"
#include "PDebug.h"

OriginalFinder::OriginalFinder(PhotoDB *db, 
                               QObject *parent):
  QObject(parent), db(db) {
  filereader = new InterruptableFileReader(this);
  rawreader = new InterruptableRawReader(this);
  connect(filereader, SIGNAL(ready(QString, InterruptableReader::Result)),
          SLOT(provide(QString, InterruptableReader::Result)));
  connect(rawreader, SIGNAL(ready(QString, InterruptableReader::Result)),
          SLOT(provide(QString, InterruptableReader::Result)));
}

OriginalFinder::~OriginalFinder() {
}

void OriginalFinder::requestOriginal(quint64 version) {
  pDebug() << "OF::requestOriginal" << version;
  requestScaledOriginal(version, PSize(0, 0));
}

PSize OriginalFinder::originalSize(quint64 vsn) {
  return db->originalSize(vsn);
}		     

void OriginalFinder::requestScaledOriginal(quint64 vsn, QSize ds) {
  pDebug() << "OF::requestScaledOriginal" << vsn << ds;
  QSqlQuery q
    = db->query("select folder, filename, filetype, width, height, orient "
                " from versions"
                " inner join photos on versions.photo==photos.id"
                " where versions.id=:a limit 1", vsn);
  ASSERT(q.next());
  quint64 folder = q.value(0).toULongLong();
  QString fn = q.value(1).toString();
  int ftype = q.value(2).toInt();
  int wid = q.value(3).toInt();
  int hei = q.value(4).toInt();
  orient = Exif::Orientation(q.value(5).toInt());
  osize = Exif::fixOrientation(PSize(wid, hei), orient);
  q.finish();
  
  QString path = db->folder(folder) + "/" + fn;
  QString ext = db->ftype(ftype);
  InterruptableReader *reader = 0;
  if (ext=="nef" || ext=="cr2")
    reader = rawreader;
  else if (ext=="jpeg" || ext=="png" || ext=="tiff")
    reader = filereader;
  ASSERT(reader);
  rawreader->cancel();
  filereader->cancel();
  desired = ds;
  version = vsn;
  filepath = path;
  reader->request(path, Exif::fixOrientation(desired, orient),
                  Exif::fixOrientation(osize, orient)); // the reader doesn't
  // know about orientation, so we need to request in file shape
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

void OriginalFinder::provide(QString fn, InterruptableReader::Result res) {
  pDebug() << "OF::provide" << fn << res.ok << res.error;
  if (fn!=filepath)
    return;
  if (!res.ok) {
    pDebug() << "OF::provide: " << res.error << " on " << fn;
    COMPLAIN("OriginalFinder: got no result for " + fn);
    return;
  }
  Image16 img = res.image;
  if (img.isNull()) {
    COMPLAIN("OriginalFinder: got null image for " + fn);
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
