// InterruptableFileReader.h

#ifndef INTERRUPTABLEFILEREADER_H

#define INTERRUPTABLEFILEREADER_H

#include "InterruptableReader.h"

class InterruptableFileReader: public InterruptableReader {
  Q_OBJECT;
public:
  InterruptableFileReader(QObject *parent=0): InterruptableReader(parent) { }
  virtual ~InterruptableFileReader() { }
protected:
  virtual QIODevice &source() { return src; }
  virtual bool openCurrent();
  virtual qint64 nextChunkSize();
  virtual bool atEnd() const;
private:
  QFile src;
  qint64 size;
};

#endif
