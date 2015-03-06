// NoResult.h

#ifndef NORESULT_H

#define NORESULT_H

class NoResult {
public:
  NoResult(QString msg="", int n=0): msg(msg), n(n) { }
public:
  QString msg;
  int n;
};

#endif
