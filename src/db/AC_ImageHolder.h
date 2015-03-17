// AC_ImageHolder.h

#ifndef AC_IMAGEHOLDER_H

#define AC_IMAGEHOLDER_H

#include <QMutex>
#include <QObject>
#include "Image16.h"
#include <QMap>

class AC_ImageHolder: public QObject {
  /* The ImageHolder can hold any number of images. This is for use by
     AutoCache and AC_Worker in processing cacheModified. The idea is
     that AutoCache will put the latest version of an image here,
     and AC_Worker will get get that version as soon as it is available.
     Older versions are simply dropped.
   */
public:
  AC_ImageHolder(QObject *parent);
  void setImage(quint64 id, Image16 img);
  /* Stores a new version of the identified image. */
  Image16 getImage(quint64 id);
  /* Retrieves the latest version of the identified image. Returns a null
     image if there is no such image or if the latest version has already
     been retrieved.
  */
private:
  QMutex mutex;
  QMap<quint64, Image16> hold;
};

#endif
