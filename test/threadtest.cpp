#include <QDebug>
#include "Image16.h"
#include "Adjustments.h"
#include "Adjuster.h"
#include <vector>
#include <future>
#include <stdint.h>

#define IMGFN "/home/wagenaar/Pictures/stuff/Sam at San Rafael 130125.jpg"

void adjthreads(Image16 const &img) {
  qDebug() << "Hello world" << img.size();
  for (int m=0; m<4; m++) {
    for (int cores=1; cores<=6; cores++) {
      Adjuster adj;
      adj.setOriginal(img);
      adj.setMaxThreads(cores);
      QTime t; t.start();
      for (int n=0; n<10; n++) {
        Adjustments set;
        set.expose = 0.1 + (n%10)*.01;
        set.black =  0.1 + (n%17)*.01;
        set.wb = 0.1 + (n%21)*.01;
        set.saturation = 0.1 + (n%19)*.01;
        set.shadows = 0.1 + (n%11)*.01;
        adj.retrieveFull(set);
      }
      qDebug() << cores << t.elapsed();
    }
  }
}

void measadj(Image16 const &img, int nthr) {
  Adjuster adj;
  adj.setOriginal(img);
  adj.setMaxThreads(nthr);
  QTime t; t.start();
  int N = 100;
  for (int n=0; n<N; n++) {
    Adjustments set;
    set.expose = 0.1 + (n%10)*.01;
    set.black =  0.1 + (n%17)*.01;
    set.wb = 0.1 + (n%21)*.01;
    set.saturation = 0.1 + (n%19)*.01;
    set.shadows = 0.1 + (n%11)*.01;
    adj.retrieveFull(set);
  }
  qDebug() << t.elapsed()*1.0/N << "ms/retrieve"
           << " at " << img.width()*img.height()/1e6 << "MPix"
           << " and " << nthr << "thread(s)";
}

void measthr() {
  int64_t N = 10000;
  std::vector< std::future<int64_t> > futures;
  auto foo = [](int64_t n) { return n+1; };
  QTime t; t.start();
  for (int64_t n=0; n<N; n++)
    futures.push_back(std::async(std::launch::async, foo, n));
  qDebug() << t.elapsed()*1000.0/N << "us/setup";
  t.start();  
  int64_t s = 0;  
  for (auto &f: futures)
    s += f.get();
  qDebug() << t.elapsed()*1000.0/N << "us/fetch" << "   ("<<s<<")";
}

int main(int, char **) {
  Image16 img(QImage(IMGFN));
  img = img.scaled(PSize(3072, 2048));
  // adjthreads(img);
  measadj(img, 1);
  measadj(img, 4);
  img = img.scaled(PSize(1536,1024));
  measadj(img, 1);
  measadj(img, 4);
  img = img.scaled(PSize(768, 512));
  measadj(img, 1);
  measadj(img, 4);
  measthr();
  
  return 0;
}
