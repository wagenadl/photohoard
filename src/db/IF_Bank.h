// IF_Bank.h

#ifndef IF_BANK_H

#define IF_BANK_H

#include <QObject>
#include "Exif.h"

class IF_Bank: public QObject {
  Q_OBJECT;
public:
  IF_Bank(int nthreads, QObject *parent=0);
  virtual ~IF_Bank();
  int availableThreads() const;
  int totalThreads() const;
  int queueLength() const;
  void findImage(quint64 id, QString path, QString ext,
		 Exif::Orientation orient, QSize ns,
		 class Sliders const &mods,
		 int maxdim, bool urgent);
signals:
  void foundImage(quint64 id, Image16 img, QSize originalSize);
  void exception(QString);
private:
  QVector<class ImageFinder *> finders;
};

#endif
