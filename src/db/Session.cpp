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
#include "FirstRunDialog.h"

QStringList Session::recentDatabases() {
  return Settings().get("recentdbs", QStringList()).toStringList();
}

Session::Session(QString dbfn0, bool create, bool readonly, QString cacheloc):
  dbfn(dbfn0), readonly(readonly) {
  active = false;
  sdb = 0;
  ac = 0;
  scan = 0;
  expo = 0;
  mw = 0;

  if (dbfn=="") {
    QStringList recent = Settings().recentFiles();
    dbfn = recent.isEmpty() ? FileLocations::defaultDBFile() : recent[0];
  }

  if (readonly)
    create = false;

  if (readonly && !QFile(dbfn).exists()) {
    ErrorDialog::fatal("No database exists at “" + dbfn
                       + "”. Cannot open read-only.");
    return;
  }

  bool isdefault = dbfn==FileLocations::defaultDBFile();
  
  if (isdefault && !QFile(dbfn).exists()) {
    create = true; // autocreate default db
  }
  
  if (create && QFile(dbfn).exists()) {
    ErrorDialog::fatal("A database already exists at “" + dbfn
                         + "”. Cannot create a new one.");
    return;
  }

  QStringList roots;
  if (create) {
    FirstRunDialog frd;
    if (!frd.exec())
      return; // never mind, I guess
    pDebug() << "Creating database at " << dbfn;
    PhotoDB::create(dbfn);
    roots = frd.roots();
  }
  
  if (!QFile(dbfn).exists()) {
    ErrorDialog::fatal("No database found at “" + dbfn + "”.");
    return;
  }

  if (!SessionDB::sessionExists(dbfn)) {
    pDebug() << "Creating session for " << dbfn << cacheloc;
    QString cachedir = cacheloc.isEmpty()
      ? FileLocations::defaultCacheDirForDB(dbfn)
      : cacheloc + "/" + FileLocations::databaseUuid(dbfn) + "-cache";
    SessionDB::createSession(dbfn, cachedir);
  }
  
  sdb = new SessionDB();
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

  Settings().markRecentFile(dbfn);

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

    if (create) {
      for (QString root: roots) 
        scan->addTree(QFileInfo(root).absoluteFilePath(), "Default");
    }
  }

  expo = new Exporter(sdb, 0);
  expo->start();
  
  mw = new MainWindow(sdb, scan, ac, expo);
  connect(mw, &MainWindow::destroyed, this, &Session::quit);
  mw->setAttribute(Qt::WA_DeleteOnClose);
  active = true;
  mw->show();
  mw->scrollToCurrent();
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
