// AllAdjustments.cpp

#include "AllAdjustments.h"
#include "PDebug.h"

class AllAdjustmentsFoo {
public:
  AllAdjustmentsFoo() {
    qRegisterMetaType<AllAdjustments>("AllAdjustments");
  }
};

static AllAdjustmentsFoo foo;


AllAdjustments::AllAdjustments() {
}

AllAdjustments AllAdjustments::fromDB(quint64 vsn, class PhotoDB &db) {
  AllAdjustments adjs;
  adjs.readFromDB(vsn, db);
  return adjs;
}

void AllAdjustments::readFromDB(quint64 vsn, class PhotoDB &db) {
  rereadFromDB(vsn, 0, db);
}

void AllAdjustments::rereadFromDB(quint64 vsn, int n0, class PhotoDB &db) {
  if (n0==0) {
    base.readFromDB(vsn, db);
    n0 = 1;
  }
  
  Layers ll(vsn, &db);
  int N = ll.count();
  layers.resize(N);
  layadj.resize(N);

  for (int n=n0; n<=N; n++)
    layers[n-1].readFromDB(vsn, n, db);
  for (int n=n0; n<=N; n++)
    layadj[n-1].readFromDB(vsn, n, db);
  pDebug() << "reread from db" << *this;
}

void AllAdjustments::writeToDB(quint64 vsn, class PhotoDB &db) const {
  base.writeToDB(vsn, db);
  int N = layers.size();
  for (int n=0; n<N; n++)
    layers[n].writeToDB(vsn, n+1, db);
}

Adjustments const &AllAdjustments::baseAdjustments() const {
  return base;
}

Adjustments &AllAdjustments::baseAdjustments() {
  return base;
}

int AllAdjustments::layerCount() const {
  return layers.size();
}

Layer const &AllAdjustments::layer(int n) const {
  Q_ASSERT(n>=1 && n<=layerCount());
  return layers[n-1];
}

void AllAdjustments::setLayer(int n, Layer const &l) {
  Q_ASSERT(n>=1 && n<=layerCount()+1);
  if (n>layerCount()) {
    layers << l;
    layadj << Adjustments();
  } else {
    layers[n-1] = l;
  }
}

Adjustments const &AllAdjustments::layerAdjustments(int n) const {
  if (n==0)
    return baseAdjustments();
  Q_ASSERT(n>=1 && n<=layerCount());
  return layadj[n-1];
}
  
Adjustments &AllAdjustments::layerAdjustments(int n) {
  if (n==0)
    return baseAdjustments();
  Q_ASSERT(n>=1 && n<=layerCount());
  return layadj[n-1];
}

bool AllAdjustments::isDefault() const {
  if (!base.isDefault())
    return false;
  for (Adjustments const &adj: layadj)
    if (!adj.isDefault())
      return false;
  return true;
}

QDebug &operator<<(QDebug &dbg, AllAdjustments const &adj) {
  dbg << "base" << adj.baseAdjustments();
  for (int k=0; k<adj.layerCount(); k++)
    dbg << "layer" << k+1 << adj.layerAdjustments(k+1);
  return dbg;
}
