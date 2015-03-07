// InterruptableFileReader.h

#ifndef INTERRUPTABLEFILEREADER_H

#define INTERRUPTABLEFILEREADER_H

#include "InterruptableReader.h"

#include <QFile>

class InterruptableFileReader: public InterruptableReader {
  Q_OBJECT;
public:
  InterruptableFileReader(QObject *parent=0);
  virtual ~InterruptableFileReader() { }
protected:
  virtual QIODevice &source() { return *src; }
  virtual bool open();
private:
  QFile *src;
};

#endif
