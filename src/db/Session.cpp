// Session.cpp

#include "Session.h"
#include "Settings.h"
#include "SessionDB.h"
#include "RunControl.h"
#include "ErrorDialog.h"
#include "PDebug.h"
#include "FileLocations.h"
#include "Scanner.h"
#include "PhotoDB.h"
#include "PurgeCache.h"
#include "Exporter.h"
#include "AutoCache.h"
#include "MainWindow.h"
#include "BasicCache.h"

QStringList Session::recentDatabases() {
  return Settings().get("recentdbs", QStringList()).toStringList();
}

Session::Session(QString dbfn0, bool create, bool readonly):
  dbfn(dbfn0), readonly(readonly) {
  active = false;
  sdb = 0;
  ac = 0;
  scan = 0;
  expo = 0;
  mw = 0;

  bool isdefault = false;
  if (dbfn=="") {
    QString defaultdbfn = FileLocations::defaultDBFile();
    dbfn = Settings().get("currentdb", defaultdbfn).toString();
    isdefault = dbfn==defaultdbfn;
  }

  if (create && QFile(dbfn).exists()) {
    ErrorDialog::fatal("A database already exists at " + dbfn
                         + ". Cannot create a new one.");
    return;
  }

  if (create || (isdefault && !QFile(dbfn).exists())) {
    pDebug() << "Creating database at " << dbfn;
    PhotoDB::create(dbfn);
  }
  
  if (!QFile(dbfn).exists()) {
    ErrorDialog::fatal("No database found at " + dbfn);
    return;
  }

  if (!SessionDB::sessionExists(dbfn)) {
    pDebug() << "Creating session for " << dbfn;
    SessionDB::createSession(dbfn, FileLocations::cacheDirForDB(dbfn));
  }
  
  sdb = new SessionDB();
  //  db.enableDebug();
  sdb->open(dbfn, readonly);

  if (!readonly) {
    // verify not already open, then store our pid
    /* Strictly speaking, this procedure should be protected by some
       locking mechanism to prevent races. Is that possible?
    */
    quint64 pid = sdb->retrievePid();
    //    qDebug() << "retrieve pid " << pid;
    if (pid && RunControl::isRunning(pid)) {
      ErrorDialog::fatal("The database “" + dbfn
                         + "” is already open in photohoard.");
      return;
    }
    sdb->storePid(RunControl::pid());
  }

  QString cachefn = sdb->cacheDirname();
  //  qDebug() << "cachedir" << sdb->cacheDirname();
  if (!QDir(cachefn).exists()) {
    pDebug() << "Creating cache at " << cachefn;
    BasicCache::create(cachefn);
  }

  ac = new AutoCache(sdb, cachefn);

  if (!sdb->isReadOnly()) {
    scan = new Scanner(sdb);
    QObject::connect(scan, SIGNAL(updated(QSet<quint64>)),
		     ac, SLOT(recache(QSet<quint64>)));
    QObject::connect(scan, SIGNAL(cacheablePreview(quint64, Image16)),
		     ac, SLOT(cachePreview(quint64, Image16)));
    scan->start();
  }

  expo = new Exporter(sdb, 0);
  expo->start();
  
  mw = new MainWindow(sdb, scan, ac, expo);
  connect(mw, &MainWindow::destroyed, this, &Session::quit);
  mw->setAttribute(Qt::WA_DeleteOnClose);
  active = true;
  mw->show();
  mw->scrollToCurrent();

  QStringList recent = recentDatabases();
  int idx = recent.indexOf(dbfn);
  if (idx!=0) {
    if (idx>0) 
      recent.removeAt(idx);
    recent.prepend(dbfn);
    while (recent.size() > 20)
      recent.removeLast();
    Settings().set("recentdbs", recent);
  } else {
    // already at head of list, nothing to do
  }
}

void Session::quit() {
  if (!active)
    return;
  active = false;

  mw = 0; // deletes itself on close
  
  if (scan) {
    scan->stopAndWait(1000);
    delete scan;
    scan = 0;
  }

  delete ac;
  ac = 0;

  expo->stop();
  delete expo;
  expo = 0;

  PurgeCache::purge(*sdb, sdb->cacheDirname());

  if (!readonly)
    sdb->storePid(0);
  sdb->close();
  delete sdb;
  sdb = 0;

  deleteLater();
}

Session::~Session() {
}

bool Session::isActive() const {
  return active;
}