// IF_Worker.h

#ifndef IF_WORKER_H

#define IF_WORKER_H

#include <QObject>
#include "Exif.h"

class IF_Worker: public QObject {
  Q_OBJECT;
public:
  IF_Worker(QObject *parent=0);
  Image16 findImageNow(QString path, QString mods, QString ext,
                      Exif::Orientation orient, int maxdim, QSize ns,
                      bool *fullSizeReturn=0);
  /* maxdim=0 means do not scale. In that case, ns can be null.
     maxdim>0 means scale to fit in a maxdim-sized rectangle. In that case,
     ns must be the natural size of the image. This can be faster for
     big raw files, because dcraw can be called with "-h" flag.
   * We can only return 8-bit images this way, which is not ideal.
   * The current implementation is not icc-profile aware. We assume that
     all jpeg files are stored with sRGB profile. This is obviously naive.
     This function will always remain naive; the caller can apply profiles.
     My intention is that the cache always uses sRGB profiles.
     Note that I do not find an 0x8773 tag ("IntercolorProfile") in any of
     my JPEG files. The 0xa001 ("ColorSpace") tag is set to enum value sRGB.
   * Currently, I don't know how to apply mods. But that will have to be
     sorted out eventually. Major question is how to apply mods rapidly.
     Probably, that will require more cleverness than this function can handle:
     it should be possible to find an image based on a parent version with
     fewer mods. This is tricky: I might need to somehow hide the original
     version when the user wants only one modified version.
   */
                                             
public slots:
  void findImage(quint64 id, QString path, QString mods, QString ext,
                 Exif::Orientation orient, int maxdim, QSize ns);
signals:
  void foundImage(quint64 id, Image16 img, bool isFullSize);
  void exception(QString);
private:
  Image16 upsideDown(Image16 &);
  Image16 rotateCW(Image16 &);
  Image16 rotateCCW(Image16 &);
private:
  class Adjuster *adjuster;
};

#endif
