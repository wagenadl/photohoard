// PhotoDB.cpp

#include "PhotoDB.h"
#include <system_error>
#include "PDebug.h"
#include "SqlFile.h"
#include <QFile>
#include "NoResult.h"

PhotoDB::PhotoDB(QString fn): Database(fn),
                              folders(new QMap<quint64, QString>),
                              ftypes(new QMap<int, QString>),
                              makes(new QMap<int, QString>),
                              models(new QMap<int, QString>),
                              lenses(new QMap<int, QString>) {
  QSqlQuery q = query("select id, version from info limit 1");
  if (!q.next())
    throw NoResult();

  pDebug() << "Opened PhotoDB " << fn
	   << ": " << q.value(0).toString() << q.value(1).toString();

  query("pragma synchronous = 0");

  q = query("select id, stdext from filetypes");
  while (q.next()) 
    (*ftypes)[q.value(0).toInt()] = q.value(1).toString();

  query("create table if not exists M.filter "
        "(version integer references versions(id), "
        " photo integer references photos(id) )");
  query("create index if not exists M.photoidx on filter(photo)");
  
}

QString PhotoDB::ftype(int ft) const {
  return (*ftypes)[ft];
}

QString PhotoDB::folder(quint64 id) {
  if (folders->contains(id))
    return (*folders)[id];

  QString folder = simpleQuery("select pathname from folders where id=:i", id)
    .toString();
  (*folders)[id] = folder;
  return folder;
}

PhotoDB PhotoDB::create(QString fn) {
  QFile f(fn);
  if (f.exists()) {
    pDebug() << "Could not create new PhotoDB: File exists: " << fn;
    throw std::system_error(std::make_error_code(std::errc::file_exists));
  }
  Database db(fn);
  SqlFile sql(":/setupdb.sql");
  QSqlQuery q(*db);
  db.beginAndLock();
  for (auto c: sql) {
    if (!q.exec(c)) {
      pDebug() << "PhotoDB: Could not setup: " << q.lastError().text();
      pDebug() << "  at " << c;
      db.rollbackAndUnlock();
      throw q;
    }
  }
  db.commitAndUnlock();
  return PhotoDB(fn);
}

quint64 PhotoDB::photoFromVersion(quint64 v) {
  return simpleQuery("select photo from versions where id==:a", v)
    .toULongLong();
}

QDateTime PhotoDB::captureDate(quint64 p) {
  return simpleQuery("select capturedate from photos where id==:a", p)
    .toDateTime();
}

PhotoDB::PhotoSize PhotoDB::photoSize(quint64 p) {
  QSqlQuery q = query("select width, height, orient from photos where id==:a",
		      p);
  if (!q.next())
    throw NoResult();
  PhotoSize ps;
  ps.filesize = PSize(q.value(0).toInt(), q.value(1).toInt());
  ps.orient = Exif::Orientation(q.value(2).toInt());
  return ps;
}

QString PhotoDB::camera(int id) {
  QString m = make(id);
  QString c = model(id);
  return m.isNull() ? c : m + " " + c;

}

QString PhotoDB::model(int id) {
  if (!models->contains(id)) 
    (*models)[id] = simpleQuery("select camera from cameras where id==:a", id)
      .toString();
  return (*models)[id];
}

QString PhotoDB::make(int id) {
  if (!makes->contains(id)) 
    (*makes)[id] = simpleQuery("select make from cameras where id==:a", id)
      .toString();
  return (*makes)[id];
}

QString PhotoDB::lens(int id) {
  if (!lenses->contains(id)) 
    (*lenses)[id] = simpleQuery("select lens from lenses where id==:a", id)
      .toString();
  return (*lenses)[id];
}


PhotoDB::VersionRecord PhotoDB::versionRecord(quint64 id) {
  VersionRecord vr;
  QSqlQuery q = query("select photo, mods, "
                      "starrating, colorlabel, acceptreject "
                      "from versions where id==:a", id);
  if (!q.next())
    throw NoResult();
  vr.id = id;
  vr.photo = q.value(0).toULongLong();
  vr.mods = q.value(1).toString();
  vr.starrating = q.value(2).toInt();
  vr.colorlabel = ColorLabel(q.value(3).toInt());
  vr.acceptreject = AcceptReject(q.value(4).toInt());
  return vr;
}

PhotoDB::PhotoRecord PhotoDB::photoRecord(quint64 id) {
  PhotoRecord pr;
  QSqlQuery q = query("select folder, filename, filetype, width, height, "
                      "camera, lens, exposetime, fnumber, focallength, "
                      "distance, iso, orient, capturedate "
                      "from photos where id==:a", id);
  if (!q.next())
    throw NoResult();
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
  pr.orient = Exif::Orientation(q.value(12).toInt());
  pr.capturedate = q.value(13).toDateTime();
  return pr;
}



void PhotoDB::setColorLabel(quint64 versionid, PhotoDB::ColorLabel label) {
  query("update versions set colorlabel=:a where id==:b",
        int(label), versionid);
}

void PhotoDB::setStarRating(quint64 versionid, int stars) {
  query("update versions set starrating=:a where id==:b",
        stars, versionid);
}
  
void PhotoDB::setAcceptReject(quint64 versionid, PhotoDB::AcceptReject label) {
  query("update versions set acceptreject=:a where id==:b",
        int(label), versionid);
}

quint64 PhotoDB::root(quint64 folderid) {
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
  return simpleQuery("select count(*) from filter"
                     " inner join photos on filter.photo=photos.id"
                     " inner join folders on photos.folder==folders.id"
                     " where folders.pathname==:a", folder).toInt();
}

int PhotoDB::countInTree(QString folder) const {
  int nsub =  simpleQuery("select count(*) from filter"
                          " inner join photos on filter.photo=photos.id"
                          " inner join folders on photos.folder==folders.id"
                          " where folders.pathname like :a", folder+"/%")
    .toInt();
  return nsub + countInFolder(folder);
}

int PhotoDB::countInDateRange(QDateTime t0, QDateTime t1) const {
  return simpleQuery("select count(*) from filter"
                     " inner join photos on filter.photo=photos.id"
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
