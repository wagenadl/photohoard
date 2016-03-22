// PhotoDB.cpp

#include "PhotoDB.h"
#include <system_error>
#include "PDebug.h"
#include "SqlFile.h"
#include <QFile>
#include <QDir>
#include "Adjustments.h"

void PhotoDB::open(QString fn) {
  Database::open(fn);

  QSqlQuery q = query("select id, version from info limit 1");
  ASSERT(q.next());
  
  query("pragma synchronous = 0");
  query("pragma foreign_keys = on");

  q = query("select id, stdext from filetypes");
  while (q.next()) 
    ftypes[q.value(0).toInt()] = q.value(1).toString();

  query("attach database ':memory:' as M");
  query("create table if not exists M.filter"
        " ( version integer, "
        "   photo integer )");
  query("create index if not exists M.photoidx on filter(photo)");
  
  query("create table if not exists M.selection"
        " ( version integer unique on conflict ignore )");
}

void PhotoDB::clone(PhotoDB const &src) {
  Database::clone(src);

  query("pragma synchronous = 0");
  query("pragma foreign_keys = on");

  ftypes = src.ftypes;
}

void PhotoDB::create(QString fn) {
  QFile f(fn);
  if (f.exists())
    CRASH("Could not create new PhotoDB: File exists: " + fn);

  if (fn.contains("/")) {
    QString parent = fn.left(fn.lastIndexOf("/"));
    QDir pdir(parent);
    if (!pdir.exists()) 
      pdir.mkpath(".");
  }

  Database db;
  db.open(fn);
  SqlFile sql(":/setupdb.sql");
  {
    Transaction t(&db);

    for (auto c: sql) 
      db.query(c);

    QString cachefn = fn;
    if (fn.endsWith(".db"))
      fn.replace(".db", ".cache");
    else
      fn += ".cache";
    db.query("insert into cachefn values (:a)", cachefn);

    t.commit();
  }
    
  db.close();
}

QString PhotoDB::ftype(int ft) const {
  return ftypes[ft];
}

quint64 PhotoDB::findFolder(QString path) const {
  if (revFolders.contains(path))
    return revFolders[path];

  QSqlQuery q = constQuery("select id from folders where pathname==:a", path);
  if (!q.next())
    return 0;
  
  quint64 id = q.value(0).toULongLong();
  revFolders[path] = id;
  return id;
}

QString PhotoDB::folder(quint64 id) const {
  if (folders.contains(id))
    return folders[id];

  QString folder = simpleQuery("select pathname from folders where id=:a", id)
    .toString();
  folders[id] = folder;
  return folder;
}


quint64 PhotoDB::photoFromVersion(quint64 v) const {
  return simpleQuery("select photo from versions where id==:a", v)
    .toULongLong();
}

QDateTime PhotoDB::captureDate(quint64 p) const {
  return simpleQuery("select capturedate from photos where id==:a", p)
    .toDateTime();
}

PSize PhotoDB::photoSize(quint64 p) const {
  QSqlQuery q
    = constQuery("select width, height from photos where id==:a", p);
  ASSERT(q.next());
  return PSize(q.value(0).toInt(), q.value(1).toInt());
}

QString PhotoDB::camera(int id) const {
  QString m = make(id);
  QString c = model(id);
  return m.isNull() ? c : m + " " + c;

}

QString PhotoDB::model(int id) const {
  if (!models.contains(id)) 
    models[id] = simpleQuery("select camera from cameras where id==:a", id)
      .toString();
  return models[id];
}

QString PhotoDB::make(int id) const {
  if (!makes.contains(id)) 
    makes[id] = simpleQuery("select make from cameras where id==:a", id)
      .toString();
  return makes[id];
}

QString PhotoDB::lens(int id) const {
  if (!lenses.contains(id)) 
    lenses[id] = simpleQuery("select lens from lenses where id==:a", id)
      .toString();
  return lenses[id];
}

QString PhotoDB::cameraAlias(int id) const {
  if (!cameraAliases.contains(id)) {
    QString alias = simpleQuery("select alias from cameras where id==:a", id)
      .toString();
    cameraAliases[id] = alias.isEmpty() ? camera(id) : alias;
  }
  return cameraAliases[id];
}

QString PhotoDB::lensAlias(int id) const {
  if (!lensAliases.contains(id)) {
    QString alias = simpleQuery("select alias from lenses where id==:a", id)
      .toString();
    lensAliases[id] = alias.isEmpty() ? lens(id) : alias;
  }
  return lensAliases[id];
}

void PhotoDB::setCameraAlias(int id, QString alias) {
  cameraAliases[id] = alias;
  query("update cameras set alias=:a where id==:b", alias, id);
}

void PhotoDB::setLensAlias(int id, QString alias) {
  lensAliases[id] = alias;
  query("update lenses set alias=:a where id==:b", alias, id);
}

PhotoDB::VersionRecord PhotoDB::versionRecord(quint64 id) const {
  VersionRecord vr;
  QSqlQuery q = constQuery("select photo,"
                           " starrating, colorlabel, acceptreject, orient"
                           " from versions where id==:a", id);
  ASSERT(q.next());
  vr.id = id;
  vr.photo = q.value(0).toULongLong();
  vr.starrating = q.value(1).toInt();
  vr.colorlabel = ColorLabel(q.value(2).toInt());
  vr.acceptreject = AcceptReject(q.value(3).toInt());
  vr.orient = Exif::Orientation(q.value(4).toInt());
  return vr;
}

PhotoDB::PhotoRecord PhotoDB::photoRecord(quint64 id) const {
  PhotoRecord pr;
  QSqlQuery q = constQuery("select folder, filename, filetype, width, height,"
                           " camera, lens, exposetime, fnumber, focallength,"
                           " distance, iso, capturedate"
                           " from photos where id==:a", id);
  ASSERT(q.next());
  pr.id = id;
  pr.folderid = q.value(0).toInt();
  pr.filename = q.value(1).toString();
  pr.filetype = q.value(2).toInt();
  pr.filesize = PSize(q.value(3).toInt(), q.value(4).toInt());
  pr.cameraid = q.value(5).toInt();
  pr.lensid = q.value(6).toInt();
  pr.exposetime_s = q.value(7).toDouble();
  pr.fnumber = q.value(8).toDouble();
  pr.focallength_mm = q.value(9).toDouble();
  pr.distance_m = q.value(10).toDouble();
  pr.iso = q.value(11).toDouble();
  pr.capturedate = q.value(12).toDateTime();
  return pr;
}

void PhotoDB::addUndoStep(quint64 versionid, QString key,
                          QVariant oldvalue, QVariant newvalue) {
  QDateTime now = QDateTime::currentDateTime();
  query("delete from undo where version==:a and undone==1", versionid);
  QSqlQuery q = query("select stepid, version,"
                      " item, oldvalue, newvalue, created"
                      " from undo"
                      " order by stepid desc limit 1");
  if (q.next()) {
    quint64 stepid = q.value(0).toULongLong();
    quint64 v = q.value(1).toULongLong();
    QString k = q.value(2).toString();
    QVariant ov = q.value(3);
    QVariant nv = q.value(4);
    QDateTime dt = q.value(5).toDateTime();
    if (v==versionid && k==key && dt.msecsTo(now)<5000) {
      // probably overwrite or cancel
      if (k==".tag") {
        if (newvalue==ov && oldvalue==nv) {
          query("delete from undo where stepid==:a", stepid);
          return;
        }
      } else {
        if (newvalue==ov) // cancel
          query("delete from undo where stepid==:a", stepid);
        else // overwrite
          query("update undo set newvalue=:a, created=:b"
                " where stepid==:c", newvalue, now, stepid);
        return;
      }
    }
  }
  query("insert into undo (version, item, oldvalue, newvalue, created)"
        " values (:a, :b, :c, :d, :e)",
        versionid, key, oldvalue, newvalue, now);
}

void PhotoDB::setColorLabel(quint64 versionid, PhotoDB::ColorLabel label) {
  QVariant old = simpleQuery("select colorlabel from versions where id==:a",
                             versionid);
  addUndoStep(versionid, ".colorlabel", old, int(label));
  query("update versions set colorlabel=:a where id==:b",
        int(label), versionid);
}

void PhotoDB::setStarRating(quint64 versionid, int stars) {
  QVariant old = simpleQuery("select starrating from versions where id==:a",
                             versionid);
  addUndoStep(versionid, ".starrating", old, stars);
  query("update versions set starrating=:a where id==:b",
        stars, versionid);
}
  
void PhotoDB::setAcceptReject(quint64 versionid, PhotoDB::AcceptReject label) {
  QVariant old = simpleQuery("select acceptreject from versions where id==:a",
                             versionid);
  addUndoStep(versionid, ".acceptreject", old, int(label));
  query("update versions set acceptreject=:a where id==:b",
        int(label), versionid);
}

quint64 PhotoDB::root(quint64 folderid) const {
  while (true) {
    bool ok;
    quint64 parentid
      = simpleQuery("select parentfolder from folders where id==:a", folderid)
      .toULongLong(&ok);
    if (ok && parentid)
      folderid = parentid;
    else
      return folderid;
  }
}

int PhotoDB::countInFolder(QString folder) const {
  quint64 id = findFolder(folder);
  if (id)
    return simpleQuery("select count(*) from filter"
                       " inner join photos on filter.photo=photos.id"
                       " where photos.folder==:a", id).toInt();
  else
    return 0;
}

int PhotoDB::countInTree(QString folder) const {
  int nsub =  simpleQuery("select count(*) from filter"
                          " inner join photos on filter.photo=photos.id"
                          " inner join folders on photos.folder==folders.id"
                          " where folders.id in "
			  " (select id from folders where pathname like :a)",
			  folder+"/%")
    .toInt();
  return nsub + countInFolder(folder);
}

bool PhotoDB::anyInTreeBelow(QString folder) const {
  if (folder=="/")
    folder = "";
  QSqlQuery q = constQuery("select 1 from filter"
                           " inner join photos on filter.photo=photos.id"
                           " inner join folders on photos.folder==folders.id"
			   " where folders.id in "
			   " (select id from folders where pathname like :a)"
			   " limit 1",
                           folder + "/%");
  return q.next();
}

int PhotoDB::countInDateRange(QDateTime t0, QDateTime t1) const {
  return simpleQuery("select count(*) from filter"
                     " inner join photos on filter.photo==photos.id"
                     " where photos.capturedate>=:a"
                     " and photos.capturedate<:b", t0, t1).toInt();
}

QDateTime PhotoDB::firstDateInRange(QDateTime t0, QDateTime t1) const {
  QSqlQuery q = constQuery("select capturedate from filter"
                           " inner join photos on filter.photo==photos.id"
                           " where capturedate>=:a and capturedate<:b"
                           " order by capturedate"
                           " limit 1", t0, t1);
  if (q.next())
    return q.value(0).toDateTime();
  else
    return QDateTime();
}

QDateTime PhotoDB::lastDateInRange(QDateTime t0, QDateTime t1) const {
  QSqlQuery q = constQuery("select capturedate from filter"
                           " inner join photos on filter.photo==photos.id"
                           " where capturedate>=:a and photos.capturedate<:b"
                           " order by capturedate desc"
                           " limit 1", t0, t1);
  if (q.next())
    return q.value(0).toDateTime();
  else
    return QDateTime();
}

QList<quint64> PhotoDB::versionsInDateRange(QDateTime t0, QDateTime t1) const {
  QSqlQuery q = constQuery("select version"
                           " from filter inner join photos"
                           " on filter.photo=photos.id"
                           " where photos.capturedate>=:a"
                           " and photos.capturedate<:b"
                           " order by photos.capturedate", t0, t1);
  QList<quint64> vv;
  while (q.next())
    vv << q.value(0).toULongLong();
  return vv;
}

QList<quint64> PhotoDB::versionsInFolder(QString folder) const {
  quint64 fid = findFolder(folder);
  if (fid==0)
    return QList<quint64>();
  
  QSqlQuery q = constQuery("select version"
                           " from filter inner join photos"
                           " on filter.photo=photos.id"
                           " where photos.folder==:a"
                           " order by photos.capturedate", fid);
  QList<quint64> vv;
  while (q.next())
    vv << q.value(0).toULongLong();
  return vv;
}

QList<QString> PhotoDB::rootFolders() const {
  QList<QString> res;
  QSqlQuery q = constQuery("select pathname from folders"
                           " where parentfolder is null"
                           " order by pathname");
  while (q.next())
    res << q.value(0).toString();
  return res;
}  

QList<QString> PhotoDB::subFolders(QString folder) const {
  if (folder=="/")
    return rootFolders();
  
  QList<QString> res;
 
  quint64 fid = findFolder(folder);
  if (fid==0)
    return res;
  
  QSqlQuery q = constQuery("select pathname from folders"
                           " where parentfolder==:a"
                           " order by leafname", fid);
  while (q.next())
    res << q.value(0).toString();
  return res;
}

quint64 PhotoDB::firstVersionInTree(QString folder) const {
  quint64 fid = findFolder(folder);
  if (fid==0)
    return 0;
  
  QSqlQuery q = constQuery("select version"
                           " from filter inner join photos"
                           " on filter.photo=photos.id"
                           " where photos.folder==:a"
                           " order by photos.capturedate limit 1", fid);
  if (q.next())
    return q.value(0).toULongLong();

  QList<QString> fff = subFolders(folder);
  for (QString f: fff) {
    quint64 v = firstVersionInTree(f);
    if (v)
      return v;
  }

  return 0;
}

quint64 PhotoDB::newVersion(quint64 vsn, bool clone) {
  VersionRecord vr = versionRecord(vsn);
  Transaction t(this);

  quint64 v1 = query("insert into versions(photo, orient, starrating,"
                     " colorlabel, acceptreject) values(:a,:b,:c,:d,:e)",
                     vr.photo, int(vr.orient), int(vr.starrating),
                     int(vr.colorlabel), int(vr.acceptreject))
    .lastInsertId().toULongLong();
  if (clone) 
    Adjustments::fromDB(vsn, *this).writeToDB(v1, *this);
  QSqlQuery q = constQuery("select tag from appliedtags where version==:a",
                           vsn);
  while (q.next())
    query("insert into appliedtags(tag, version) values(:a, :b)",
          q.value(0), v1);
  
  t.commit();
  return v1;
}

void PhotoDB::setCurrent(quint64 vsn) {
  if (vsn>0)
    query("update current set version=:a", vsn);
  else
    query("update current set version=null");
}

quint64 PhotoDB::current() const {
  return simpleQuery("select version from current").toULongLong();
}
  
PSize PhotoDB::originalSize(quint64 vsn) const {
  QSqlQuery q = constQuery("select width, height, orient"
                          " from versions"
                          " inner join photos on versions.photo==photos.id"
                          " where versions.id==:a limit 1", vsn);
  ASSERT(q.next());
  PSize s(q.value(0).toInt(), q.value(1).toInt());
  return Exif::fixOrientation(s, Exif::Orientation(q.value(2).toInt()));
}		     

PSize PhotoDB::originalSize(PhotoDB::VersionRecord const &vr,
			    PhotoDB::PhotoRecord const &pr) {
  PSize s(pr.filesize);
  return Exif::fixOrientation(s, vr.orient);
}

bool PhotoDB::hasSiblings(quint64 vsn) {
  int n = simpleQuery("select count(1) from versions where photo in"
                      " (select photo from versions where id==:a", vsn)
    .toInt();
  return n>1;
}

void PhotoDB::deleteVersion(quint64 vsn) {
  query("delete from versions where id==:a", vsn);
}

void PhotoDB::deletePhoto(quint64 vsn) {
  query("delete from photos where id==:a", vsn);
}

QSet<quint64> PhotoDB::versionsForPhoto(quint64 photo) {
  QSet<quint64> res;
  QSqlQuery q = constQuery("select id from versions where photo==:a", photo);
  while (q.next())
    res.insert(q.value(0).toULongLong());
  return res;
}

QString PhotoDB::cacheFilename() const {
  return simpleQuery("select fn from cachefn").toString();
}
