// IF_bank.cpp

#include "IF_Bank.h"
#include "ImageFinder.h"
#include "PDebug.h"

IF_Bank::IF_Bank(int nthreads, QObject *parent): QObject(parent) {
  setObjectName("IF_Bank");
  for (int n=0; n<nthreads; n++) 
    finders << new ImageFinder(this);

  for (auto f: finders)
    connect(f, SIGNAL(foundImage(quint64, Image16, QSize)),
            this, SIGNAL(foundImage(quint64, Image16, QSize)));
}

IF_Bank::~IF_Bank() {
}

int IF_Bank::totalThreads() const {
  return finders.size();
}

int IF_Bank::availableThreads() const {
  int av=0;
  for (auto f: finders)
    if (f->queueLength()==0)
      av++;
  return av;
}

int IF_Bank::queueLength() const {
  int ql=0;
  for (auto f: finders)
    ql += f->queueLength();
  return ql;
}

void IF_Bank::findImage(quint64 id, QString path, QString ext,
			Exif::Orientation orient, QSize ns,
			Sliders const &mods,
			int maxdim, bool urgent) {
  ImageFinder *f0 = 0;
  int ql0 = 0;
  for (auto f: finders) {
    int ql = f->queueLength();
    if (f0==0 || ql<ql0) {
      f0 = f;
      ql0 = ql;
    }
  }
  f0->findImage(id, path, ext, orient, ns, mods, maxdim, urgent);
}
