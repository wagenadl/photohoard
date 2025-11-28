// Collector.h

#ifndef COLLECTOR_H

#define COLLECTOR_H

#include <QThread>
#include <QUrl>
#include <QStringList>
#include "DateTime.h"

class Collector: public QThread {
  Q_OBJECT;
public:
  Collector(QObject *parent=0);
  virtual ~Collector();
  QStringList const &imageFiles() const; // do not touch unless complete
  QStringList const &movieFiles() const; // do not touch unless complete
  bool isComplete() const;
  int preliminaryCount() const;
  int preliminaryMovieCount() const;
  DateTime firstDate() const;
  DateTime lastDate() const;
public slots:
  void collect(QList<QUrl> urls);
  void cancel();
signals:
  void progress(int ntotal, int nmov, bool complete);
  void complete();
  void canceled();
private:
  virtual void run() override;
  void updateDates(DateTime const &dt);
private:
  QList<QUrl> urls;
  bool cancel_;
  bool complete_;
  QStringList imgFiles;
  QStringList movFiles;
  int cnt;
  int movcnt;
  DateTime dt0, dt1;
};

#endif
