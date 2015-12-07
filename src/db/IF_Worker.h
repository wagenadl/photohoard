// IF_Worker.h

#ifndef IF_WORKER_H

#define IF_WORKER_H

#include <QObject>
#include "Exif.h"
#include "Sliders.h"

/* Class: IF_Worker

   Worker thread for <ImageFinder>
*/
class IF_Worker: public QObject {
  Q_OBJECT;
public:
  // Constructor: IF_Worker
  IF_Worker(QObject *parent=0);

  /* Function: findImageNow
     Implementation of <ImageFinder::findImage>.

     This function actually
     loads the image file and applies the needed adjustments.

     Arguments:
     urgent - True to avoid renicing dcraw subprocess
   */
  Image16 findImageNow(QString path, QString ext,
		       Exif::Orientation orient, PSize ns,
		       class Sliders const &mods,
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
  /* Function: findImage (slot)
     Implementation of <ImageFinder::findImage>.

     This loads the image, applies adjustments, and emits the <foundImage>
     signal.
   */
  void findImage(quint64 id, QString path, QString ext,
		 Exif::Orientation orient, QSize ns,
		 Sliders mods,
		 int maxdim, bool urgent);
  
signals:
  /* Function: foundImage (signal)

     Emitted in response to <findImage> once the image has been loaded
     and adjusted as requested.
  */
  void foundImage(quint64 id, Image16 img, QSize originalSize);

  /* Function: exception (signal)
     Emitted when the worker thread catches an exception.

     At the moment, all exceptions are fatal. This mechanism is used because
     in Qt, subthreads and slots are not allowed to throw exceptions.

     Arguments:
     msg - Error message associated with the exception.
  */
  void exception(QString);
private:
  class Adjuster *adjuster;
};

#endif
