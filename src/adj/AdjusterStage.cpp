// AdjusterStage.cpp

#include "AdjusterStage.h"
#include <QDebug>
#include <vector>
#include <future>
#include <algorithm>

bool AdjusterStage::isDefault(Adjustments const &a, QStringList fields) {
  for (auto s: fields) 
    if (!a.isDefault(s))
      return false;
  return true;
}

bool AdjusterStage::isEquivalent(Adjustments const &a, Adjustments const &b,
				 QStringList fields) {
  for (auto s: fields) 
    if (a.get(s) != b.get(s))
      return false;
  return true;
}

void AdjusterStage::doapply(std::function<void(quint16*,int)> foo,
                            quint16 *data, int X, int Y, int DL) {
  qDebug() << "AS::doapply" << X << Y << maxthreads;
  if (maxthreads>1 && X*Y>1000000) {
    std::vector< std::future<void> > futures;
    int linesperrun = (Y + maxthreads - 1) / maxthreads;
    while (Y>0) {
      int now = std::min(linesperrun, Y);
      futures.push_back(std::async(std::launch::async, foo, data, now));
      data += (3*X + DL)*now;
      Y -= now;
    }
    for (auto &f: futures)
      f.get();
  } else {
    foo(data, Y);
  }
}
  
