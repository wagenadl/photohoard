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
public slots:
  void findImage(quint64 id, QString path, QString mods, QString ext,
                 Exif::Orientation orient, int maxdim, QSize natsize);
signals:
  void foundImage(quint64 id, Image16 img, bool isFullSize);
  void exception(QString);
private:
  QVector<class ImageFinder *> finders;
};

#endif
