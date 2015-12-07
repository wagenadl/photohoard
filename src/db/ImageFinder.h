// ImageFinder.h

#ifndef IMAGEFINDER_H

#define IMAGEFINDER_H

#include <QObject>
#include <QThread>
#include "Exif.h"
#include "Sliders.h"

/* Class: ImageFinder

   Given a version ID, pathname, and adjustments, ImageFinder can load an
   image and apply the adjustments in a asynchronous manner.

   It is worth noting that ImageFinder has no connection to the <PhotoDB>.
*/
class ImageFinder: public QObject {
  Q_OBJECT;
public:
  /* Constructor: ImageFinder
   */
  ImageFinder(QObject *parent=0);
  
  virtual ~ImageFinder();

  /* Function: queueLength
     Returns number of outstanding requests for this ImageFinder.
  */
  int queueLength() const { return queuelength; }

  /* Function: findImage

     Adds a new request to the list of requests. Ultimately, the request
     will result in the emission of a single <foundImage> signal, unless
     an <exception> occurs.

     Arguments:
     id - version ID
     path - full pathname of original photo file
     ext - filetype for the photo file (e.g., "jpeg", "nef", etc.).
     orient - orientation correction
     ns - natural size of the image in the photo file (not
          orientation-corrected, but as in file)
     mods - collection of adjustments to be applied
     maxdim - largest output size requested (requesting a smaller size than
              the natural size may sometimes result in faster loading)
     urgent - true if this request should be prioritized
   */
  void findImage(quint64 id, QString path, QString ext,
		 Exif::Orientation orient, QSize ns,
		 class Sliders const &mods,
		 int maxdim, bool urgent);
  
signals:
  /* Function: foundImage (signal)

     Emitted when a request posted with <findImage> completes.

     Arguments:
     id - version ID
     img - image as loaded
     fullSize - size of the image if it would be loaded with infinite maxdim.
  */
  void foundImage(quint64 id, Image16 img, QSize fullSize);

  /* Function: exception (signal)
     Emitted when the worker thread catches an exception.

     At the moment, all exceptions are fatal. This mechanism is used because
     in Qt, subthreads and slots are not allowed to throw exceptions.

     Arguments:
     msg - Error message associated with the exception.
  */
  void exception(QString);

private slots:
  /* Function: handleFoundImage (slot)

     Normally connected to the <IF_Worker::foundImage> signal, this
     causes our <foundImage> to be emitted.
  */
  void handleFoundImage(quint64 id, Image16 img, QSize originalSize);
  
signals:  // private
  /* Function: forwardFindImage (signal)

     Passes signals connected to our <findImage> slot to the
     <IF_Worker::findImage> slot.
  */
  void forwardFindImage(quint64 id, QString path, QString ext,
			Exif::Orientation orient, QSize ns,
			Sliders mods,
			int maxdim, bool urgent);
private:
  QThread thread;
  class IF_Worker *worker;

  // Variable: queuelength
  // Keeps track of how many requested images have been not yet been found.
  //
  // Note that this count gets messed up in case of exceptions.
  int queuelength;
};

#endif
