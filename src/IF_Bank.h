// IF_Bank.h

#ifndef IF_BANK_H

#define IF_BANK_H

#include <QObject>
#include "Exif.h"

class IF_Bank: public QObject {
public:
  IF_Bank(int nthreads, QObject *parent=0);
  virtual ~IF_Bank();
  int availableThreads() const;
  int queueLength() const;
  void setMaxDim(int);
public slots:
  void findImage(quint64 id, QString path, int ver, QString ext,
                 Exif::Orientation orient);
signals:
  void foundImage(quint64 id, QImage img);
private:
  QVector<class ImageFinder *> finders;
};

#endif
