// IF_Worker.h

#ifndef IF_WORKER_H

#define IF_WORKER_H

#include <QObject>
#include "Exif.h"

class IF_Worker: public QObject {
  Q_OBJECT;
public:
  IF_Worker(QObject *parent=0);
  Image16 findImageNow(QString path, QString ext,
		       Exif::Orientation orient, PSize ns,
		       QString mods,
		       int maxdim, bool urgent,
		       PSize *fullSizeReturn=0);
  /* Scale to fit in a maxdim-sized rectangle.
     ns may be the natural size of the image. This can be faster for
     big raw files, because dcraw can be called with "-h" flag.
   * The current implementation is not icc-profile aware. We assume that
     all jpeg files are stored with sRGB profile. This is obviously naive.
     This function will always remain naive; the caller can apply profiles.
     My intention is that the cache always uses sRGB profiles.
     Note that I do not find an 0x8773 tag ("IntercolorProfile") in any of
     my JPEG files. The 0xa001 ("ColorSpace") tag is set to enum value sRGB.
   */
                                             
public slots:
  void findImage(quint64 id, QString path, QString ext,
		 Exif::Orientation orient, PSize ns,
		 QString mods,
		 int maxdim, bool urgent);
signals:
  void foundImage(quint64 id, Image16 img, PSize originalSize);
  void exception(QString);
private:
  class Adjuster *adjuster;
};

#endif
