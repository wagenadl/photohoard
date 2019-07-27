// IF_Bank.h

#ifndef IF_BANK_H

#define IF_BANK_H

#include <QObject>
#include "Exif.h"

/* Class: IF_Bank

   A collection of <ImageFinders> that can spread requests across multiple
   threads. In practice, IF_Bank may be used just like a lone <ImageFinder>.
 */
class IF_Bank: public QObject {
  Q_OBJECT;
public:
  /* Constructor: IF_Bank

     Arguments:
     nthreads - Number of threads to create
   */
  IF_Bank(int nthreads, QObject *parent=0);
  virtual ~IF_Bank();

  // Function: availableThreads
  // Returns number of threads that are not currently busy
  int availableThreads() const;

  // Function: totalThreads
  // Returns total number of threads created
  int totalThreads() const;

  // Function: queueLength
  // Returns total number of outstanding requests (across all threads)
  int queueLength() const;

  // Function: findImage
  // See <ImageFinder::findImage> for details.
  void findImage(quint64 id, QString path, QString ext,
		 Exif::Orientation orient, QSize ns,
		 class AllAdjustments const &mods,
		 int maxdim, bool urgent);

signals:
  // Function: foundImage (signal)
  // See <ImageFinder::foundImage> for details.
  void foundImage(quint64 id, Image16 img, QSize originalSize);

  // Function: exception (signal)
  // See <ImageFinder::exception> for details
  void exception(QString);
private:
  QVector<class ImageFinder *> finders;
};

#endif
