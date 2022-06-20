// Session.h

#ifndef SESSION_H

#define SESSION_H

#include <QObject>
#include <QStringList>

class Session: public QObject {
  Q_OBJECT;
public:
  static QStringList recentDatabases();
public:
  Session(QString dbfn=QString(), bool create=false, bool readonly=false);
  virtual ~Session();
  bool isActive() const;
public slots:
  void quit();
private:
  QString dbfn;
  bool readonly;
  bool active;
  class SessionDB *sdb;
  class AutoCache *ac;
  class Scanner *scan;
  class Exporter *expo;
  class MainWindow *mw;
};

#endif
