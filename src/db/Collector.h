// Collector.h

#ifndef COLLECTOR_H

#define COLLECTOR_H

#include <QThread>
#include <QUrl>
#include <QStringList>
#include <QDateTime>

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
  QDateTime firstDate() const;
  QDateTime lastDate() const;
public slots:
  void collect(QList<QUrl> urls);
  void cancel();
signals:
  void progress(int ntotal, int nmov, bool complete);
  void complete();
  void canceled();
private:
  virtual void run() override;
  void updateDates(QDateTime dt);
private:
  QList<QUrl> urls;
  bool cancel_;
  bool complete_;
  QStringList imgFiles;
  QStringList movFiles;
  int cnt;
  int movcnt;
  QDateTime dt0, dt1;
};

#endif
