// InterruptableRawReader.h

#ifndef INTERRUPTABLERAWREADER_H

#define INTERRUPTABLERAWREADER_H

#include "InterruptableReader.h"

class InterruptableRawReader: public InterruptableReader {
  Q_OBJECT;
public:
  InterruptableRawReader(QObject *parent=0);
  virtual ~InterruptableRawReader();
protected:
  virtual QIODevice &tSource();
  virtual bool uOpen();
  virtual void lPrepSource(QString fn, QSize rq, QSize ori);
  virtual void lUnprepSource();
  virtual void abort();
private:
  class QProcess *src;
  QString fn;
  QSize rqs, oris;
};

#endif
