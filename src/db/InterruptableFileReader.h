// InterruptableFileReader.h

#ifndef INTERRUPTABLEFILEREADER_H

#define INTERRUPTABLEFILEREADER_H

#include "InterruptableReader.h"

class InterruptableFileReader: public InterruptableReader {
  Q_OBJECT;
public:
  InterruptableFileReader(QObject *parent=0);
  virtual ~InterruptableFileReader();
protected:
  virtual QIODevice &tSource();
  virtual bool uOpen();
  virtual void lPrepSource(QString fn, QSize, QSize);
  virtual int uEstimateSize();
  virtual void lUnprepSource();
private:
  class QFile *src;
};

#endif
