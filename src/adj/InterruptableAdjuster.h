// InterruptableAdjuster.h

#ifndef INTERRUPTABLEADJUSTER_H

#define INTERRUPTABLEADJUSTER_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include "Adjustments.h"
#include "Image16.h"

class InterruptableAdjuster: public QThread {
  /* INTERRUPTABLEADJUSTER - Thread wrapper around ADJUSTER
     INTERRUPTABLEADJUSTER contains several /requestXXX/
     methods that function just as ADJUSTER's /retrieveXXX/
     methods, except that the methods return immediately
     and that results are later reported through the READY signal.
     Requests are not queued: if a new request is made before
     the previous one completes, the older request will be
     canceled.
   */
  Q_OBJECT;
public:
  InterruptableAdjuster(QObject *parent=0);
  // CONSTRUCTOR
  virtual ~InterruptableAdjuster();
  void requestFull(Adjustments const &settings);
  // REQUESTFULL - See Adjuster's RETRIEVEFULL
  void requestReduced(Adjustments const &settings, PSize maxSize);
  // REQUESTREDUCED - See Adjuster's RETRIEVEREDUCED
  void requestROI(Adjustments const &settings, QRect roi);
  // REQUESTROI - See ADJUSTER's RETRIEVEROI
  void requestReducedROI(Adjustments const &settings, QRect roi, PSize maxSize);
  // REQUESTREDUCEDROI - See ADJUSTER's RETRIEVEREDUCEDROI
  void cancelRequest();
  /* CANCELREQUEST - Cancel outstanding request
     No READY signal will be emitted after CANCELREQUEST
     returns.
   */
  void clear();
  /* CLEAR - Drop original image
     Also cancels any outstanding request.
  */
  PSize maxAvailableSize(Adjustments const &);
  // MAXAVAILABLESIZE - See ADJUSTER's MAXAVAILABLESIZE
  bool isEmpty();
  // ISEMPTY - See ADJUSTER's ISEMPTY
  void setOriginal(Image16 img);
  /* SETORIGINAL - Loads a new original image into the adjuster
     SETORIGINAL(img) loads IMG as the new original into
     the adjuster, canceling any outstanding request.
  */
  void setReduced(Image16 img, PSize osize);
  /* SETREDUCED - Loads a new original image into the adjuster at less than full size
     SETREDUCED(img, osize) loads a new original image into the
     adjuster, just like SETORIGINAL, but not at its full
     resolution.
     See ADJUSTER's SETREDUCED for more details.
     Technical note: OSIZE may be null, in which case IMG
     is assumed to be the full original image.
  */
signals:
  void ready(Image16 img);
  /* READY - Emitted once retrieval has finished
     READY(img) is emitted when calculation of the final
     image has been completed.
     READY() is /not/ emitted if the request was canceled.
     Note that "ready" does not imply success: IMG can be null.
     This happens, e.g., if a full size version is requested
     but only a reduced original has been provided.

     Due to the nature of mutexes, it is possible that READY() is
     emitted before /requestXXX/ returns. Using a queued connection
     prevents this. (Or at least ensures that the signal is not
     /received/ before /requestXXX/ returns.) It is even possible,
     though unlikely, that READY() is emitted after CANCELREQUEST() has been
     called, though not after it returns. This cannot be
     prevented. Connection queueing can even cause this signal to be
     /received/ after CANCELREQUEST() returns.

     The important thing to note is that if you use queued
     connections, you can never be quite sure that the READY() signal
     you receive truly corresponds to your latest request: After all,
     it could have been emitted before your latest request.
  */
protected:
  void start();
  void stop();
  virtual void run();
private:
  void handleNewRequest();
  void handleNewImage();
private:
  class Adjuster *adjuster;
  QMutex mutex;
  QWaitCondition waitcond;
  bool cancel, newreq, clear_;
  Adjustments rqAdjustments;
  QRect rqRect;
  PSize rqSize;
  bool stopsoon;
  bool empty;
  PSize scaledOSize;
  Image16 newOriginal;
  PSize oSize;
};

#endif
