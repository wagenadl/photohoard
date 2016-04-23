// ImportJob.h

#ifndef IMPORTJOB_H

#define IMPORTJOB_H

#include <QObject>
#include "CopyIn.h"

class ImportJob: public QObject {
  Q_OBJECT;
public:
  enum Operation {
    Import,
    Incorporate
  };
public:
  ImportJob(class SessionDB *db,
	    class Scanner *scanner,
	    QList<QUrl> const &sources,
	    QObject *parent=0);
  virtual ~ImportJob();
signals:
  void progress(int n); // for copying
  void complete(QString errmsg);
public slots:
  void setDestination(QString);
  void setAutoDestination();
  void setMovieDestination(QString);
  void setSourceDisposition(CopyIn::SourceDisposition);
  void setCollection(QString);
  void countSources();
  void authorizeCopy();
  void authorizeIncorporate();
  void cancel();
public:
  QString destination() const;
  bool isAutoDestination() const;
  QString movieDestination() const;
  CopyIn::SourceDisposition sourceDisposition() const;
  QString collection() const;
public:
  bool isAuthorized() const;
  bool isComplete() const;
public:
  bool hasSourceCount() const;
  int sourceCount() const; // -1 if not available; will wait if needed
  bool sourceIsExternalMedia() const;
  //  bool sourceIsWritableLocation() const;
  //  bool sourceIsSingleFolder() const;
  //  bool sourceInvolvesOnlyFiles() const;
private slots:
  void markSourceCount();
private:
  void startCopy();
private:
  class SessionDB *db;
  class Scanner *scanner;
  class CopyIn *copyin;
  class Collector *collector;
  bool authorized;

};

#endif
