// AC_ImageHolder.h

#ifndef AC_IMAGEHOLDER_H

#define AC_IMAGEHOLDER_H

#include <QMutex>
#include <QObject>
#include "Image16.h"
#include <QMap>

/* Class: AC_ImageHolder
   
   The ImageHolder can hold any number of images. This is for use by
   <AutoCache> and <AC_Worker> in processing <AutoCache::cacheModified>.
   The idea is that AutoCache will put the latest version of an image here,
   and AC_Worker can retrieve that version.
   Older versions are simply dropped.

   All methods are protected by a mutex and can safely be used across
   multiple threads.
*/
class AC_ImageHolder: public QObject {
public:
  AC_ImageHolder(QObject *parent);

  /* Function: setImage
     Store an image in the holder. Replace any image previously associated
     with the given ID.

     Arguments:
     id - version ID of image to be stored
     img - actual image
  */
  void setImage(quint64 id, Image16 img);

  /* Function: dropImage
     Remove an image from the holder. If the image was not contained in
     the holder, do nothing.

     Argument:
     id - version ID of image to be removed
  */
  void dropImage(quint64 id);

  /* Function: getImage

     Retrieves the latest version of the identified image. Returns a null
     image if there is no such image. Images can only be retrieved once;
     if the latest version has already been retrieved, this function returns
     null.

     Argument:
     id - version ID of image to retrieve
  */
  Image16 getImage(quint64 id);
private:
  QMutex mutex;
  QMap<quint64, Image16> hold;
};

#endif
