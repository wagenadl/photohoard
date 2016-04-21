// Collector.h

#ifndef COLLECTOR_H

#define COLLECTOR_H

#include <QThread>
#include <QUrl>
#include <QStringList>

class Collector: public QThread {
  Q_OBJECT;
public:
  Collector(QObject *parent=0);
  virtual ~Collector();
  QStringList const &imageFiles() const;
  QStringList const &movieFiles() const;
public slots:
  void collect(QList<QUrl> urls);
  void cancel();
signals:
  void progress(int nimg, int nmov);
  void complete();
  void canceled();
private:
  virtual void run() override;
private:
  QList<QUrl> urls;
  bool cancel_;
  QStringList imgFiles;
  QStringList movFiles;
};

#endif
