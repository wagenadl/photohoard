// ImageFinder.h

#ifndef IMAGEFINDER_H

#define IMAGEFINDER_H

#include <QObject>
#include <QThread>
#include "Exif.h"

class ImageFinder: public QObject {
  Q_OBJECT;
public:
  ImageFinder(QObject *parent=0);
  virtual ~ImageFinder();
  int queueLength() const { return queuelength; }
public slots:
  void findImage(quint64 id, QString path, QString ext,
		 Exif::Orientation orient, QSize ns,
		 QString mods,
		 int maxdim, bool urgent);
signals:
  void foundImage(quint64, Image16, bool);
  void exception(QString);
private slots:
  void handleFoundImage(quint64 id, Image16 img, QSize originalSize);
signals:  // private
  void forwardFindImage(quint64 id, QString path, QString ext,
			Exif::Orientation orient, QSize ns,
			QString mods,
			int maxdim, bool urgent);
private:
  QThread thread;
  class IF_Worker *worker;
  int queuelength;
};

#endif
