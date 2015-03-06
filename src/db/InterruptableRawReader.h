// InterruptableRawReader.h

#ifndef INTERRUPTABLERAWREADER_H

#define INTERRUPTABLERAWREADER_H

#include "InterruptableReader.h"
#include <QProcess>

class InterruptableRawReader: public InterruptableReader {
  Q_OBJECT;
public:
  InterruptableRawReader(QObject *parent=0): InterruptableReader(parent) { }
  virtual ~InterruptableRawReader() { }
protected:
  virtual QIODevice &source() { return src; }
  virtual bool openCurrent();
  virtual void stopSource();
  virtual bool atEnd() const;
private:
  QProcess src;
};

#endif
