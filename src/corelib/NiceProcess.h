// NiceProcess.h

#ifndef NICEPROCESS_H

#define NICEPROCESS_H

#include <QProcess>

class NiceProcess: public QProcess {
public:
  NiceProcess();
  void renice(int nice);
protected:
  //virtual void setupChildProcess();
private:
  int nice;
};

#endif
