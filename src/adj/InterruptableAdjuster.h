// InterruptableAdjuster.h

#ifndef INTERRUPTABLEADJUSTER_H

#define INTERRUPTABLEADJUSTER_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include "AllAdjustments.h"
#include "Image16.h"

class InterruptableAdjuster: public QThread {
  /* INTERRUPTABLEADJUSTER - Thread wrapper around ALLADJUSTER
     INTERRUPTABLEADJUSTER contains several /requestXXX/
     methods that function just as ALLADJUSTER's /retrieveXXX/
     methods, except that the methods return immediately
     and that results are later reported through the READY signal.
     Requests are not queued: if a new request is made before
     the previous one completes, the older request will be
     canceled.
     One important difference is that most of our functions take an
     additional ID argument that is an arbitrary number intended to make
     sure that the READY signal corresponds to the intended request.
   */
  Q_OBJECT;
public:
  InterruptableAdjuster(QObject *parent=0);
  // CONSTRUCTOR
  virtual ~InterruptableAdjuster();
  void requestFull(AllAdjustments const &settings, quint64 id);
  // REQUESTFULL - See Adjuster's RETRIEVEFULL
  void requestReduced(AllAdjustments const &settings,
		      PSize maxSize, quint64 id);
  // REQUESTREDUCED - See Adjuster's RETRIEVEREDUCED
  void requestROI(AllAdjustments const &settings, QRect roi, quint64 id);
  // REQUESTROI - See ADJUSTER's RETRIEVEROI
  void requestReducedROI(AllAdjustments const &settings,
			 QRect roi, PSize maxSize, quint64 id);
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
  PSize maxAvailableSize(Adjustments const &, quint64 id);
  // MAXAVAILABLESIZE - See ADJUSTER's MAXAVAILABLESIZE
  bool isEmpty();
  // ISEMPTY - See ADJUSTER's ISEMPTY
  void setOriginal(Image16 img, quint64 id);
  /* SETORIGINAL - Loads a new original image into the adjuster
     SETORIGINAL(img) loads IMG as the new original into
     the adjuster, canceling any outstanding request.
  */
  void setReduced(Image16 img, PSize osize, quint64 id);
  /* SETREDUCED - Loads a new original image at less than full size
     SETREDUCED(img, osize) loads a new original image into the
     adjuster, just like SETORIGINAL, but not at its full
     resolution.
     See ADJUSTER's SETREDUCED for more details.
     Technical note: OSIZE may be null, in which case IMG
     is assumed to be the full original image.
  */
signals:
  void ready(Image16 img, quint64 id, QSize fullsize);
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
     though unlikely, that READY() is emitted after CANCELREQUEST()
     has been called, though not after it returns. This cannot be
     prevented. Connection queuing can even cause this signal to be
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
  Image16 hnrFull(AllAdjustments const &sli);
  Image16 hnrReduced(AllAdjustments const &sli, PSize s);
private:
  class AllAdjuster *adjuster;
  QMutex mutex;
  QWaitCondition waitcond;
  bool cancel, newreq, clear_;
  AllAdjustments rqAdjustments;
  QRect rqRect;
  PSize rqSize;
  quint64 rqId;
  bool stopsoon;
  bool empty;
  PSize scaledOSize;
  Image16 newOriginal;
  PSize oSize;
  quint64 oId;
};

#endif
