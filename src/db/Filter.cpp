// Filter.cpp

#include "Filter.h"
#include "PhotoDB.h"
#include "Tags.h"
#include "PDebug.h"

Filter::Filter() {
  hascollection = false;
  hascolorlabels = false;
  hasstarrating = false;
  minstars = 0;
  maxstars = 5;
  hasstatus = false;
  statusaccepted = statusrejected = statusunset = false;
  hascamera = false;
  hasdaterange = false;
  startdate = QDate(1980, 1, 1);
  enddate = QDate(QDate::currentDate().year(), 12, 31);
  hasfilelocation = false;
  hastags = false;
}

void Filter::reset() {
  *this = Filter();
}

void Filter::setCollection(QString s) {
  hascollection = true;
  collection_ = s;
}

void Filter::unsetCollection() {
  hascollection = false;
}

void Filter::setColorLabels(QSet<PhotoDB::ColorLabel> cc) {
  hascolorlabels = true;
  colorlabels = cc;
}

void Filter::unsetColorLabels() {
  hascolorlabels = false;
}

bool Filter::includesColorLabel(PhotoDB::ColorLabel c) const {
  return colorlabels.contains(c);
}

void Filter::setStarRating(int min, int max) {
  hasstarrating = true;
  minstars = min;
  maxstars = max;
}

void Filter::unsetStarRating() {
  hasstarrating = false;
}

void Filter::setStatus(bool accepted, bool unset, bool rejected) {
  hasstatus = true;
  statusaccepted = accepted;
  statusunset = unset;
  statusrejected = rejected;
}

void Filter::unsetStatus() {
  hasstatus = false;
}

void Filter::setCamera(QString make, QString model, QString lens) {
  hascamera = true;
  cameramake = make;
  cameramodel = model;
  cameralens = lens;
}

void Filter::unsetCamera() {
  hascamera = false;
}

void Filter::setDateRange(QDate start, QDate end) {
  hasdaterange = true;
  startdate = start;
  enddate = end;
}

void Filter::unsetDateRange() {
  hasdaterange = false;
}

void Filter::setFileLocation(QString s) {
  hasfilelocation = true;
  filelocation = s;
}

void Filter::unsetFileLocation() {
  hasfilelocation = false;
}

void Filter::setTags(QStringList ss) {
  hastags = true;
  tags_.clear();
  for (auto s: ss)
    tags_ << Tags::normalCase(s.simplified());
}

void Filter::unsetTags() {
  hastags = false;
}

int Filter::count(class PhotoDB &db) const {
  return db.simpleQuery("select count(*) from versions "
			+ joinClause()
			+ " where " + whereClause(db)).toInt();
}

QString Filter::joinClause() const {
  QStringList joins;
  //  if (hasdaterange || hasfilelocation || hascamera)
  joins << "inner join photos on versions.photo==photos.id";
  return joins.join(" ");
}

QString Filter::whereClause(class PhotoDB &db) const {
  QStringList clauses;
  if (hascollection)
    clauses << collectionClause(db);
  if (hascolorlabels)
    clauses << colorLabelClause();
  if (hasstarrating)
    clauses << starRatingClause();
  if (hasstatus)
    clauses << statusClause();
  if (hascamera)
    clauses << cameraClause(db);
  if (hasdaterange)
    clauses << dateRangeClause();
  if (hasfilelocation)
    clauses << fileLocationClause(db);
  if (hastags)
    clauses << tagsClause(db);
  pDebug() << clauses.join(" AND ");
  if (clauses.isEmpty())
    return "1>0";
  else
    return clauses.join(" and ");
}

QString Filter::collectionClause(PhotoDB &db) const {
  QSqlQuery q = db.query("select id from tags where tag==:a", "Collections");
  if (!q.next())
    return collection_.isEmpty() ? "1>0" : "0>1";
  int colparent = q.value(0).toInt();
  if (collection_.isEmpty()) {
    return "versions.id not in ( select version from appliedtags where tag in "
      "( select id from tags where parent==" + QString::number(colparent)
      +") )";
  } else {
    q = db.query("select id from tags where tag==:a and parent==:b",
                 collection_, colparent);
    if (q.next())
      return "versions.id in ( select version from appliedtags where tag=="
	+ QString::number(q.value(0).toInt()) + " )";
    else
      return "0>1";
  }
}

QString Filter::colorLabelClause() const {
  if (colorlabels.size()==0)
    return "0>1";
  if (colorlabels.size()==1) 
    return "colorlabel==" + QString::number(int(*colorlabels.begin()));
  QStringList cc;
  for (PhotoDB::ColorLabel c: colorlabels)
    cc << QString::number(int(c));
  return "colorlabel in (" + cc.join(", ") + ")";
}

QString Filter::starRatingClause() const {
  QStringList bits;
  if (minstars>0)
    bits << "starrating>" + QString::number(minstars);
  if (maxstars<5)
    bits << "starrating<" + QString::number(maxstars);
  if (bits.isEmpty())
    return "1>0";
  else
    return bits.join(" and ");
}

QString Filter::statusClause() const {
  if (statusaccepted || statusunset || statusrejected) {
    QStringList vv;
    if (statusaccepted)
      vv << "1";
    if (statusunset)
      vv << "0";
    if (statusrejected)
      vv << "-1";
    return "acceptreject in ( " + vv.join(", ") + " )";
  } else {
    return "0>1";
  }
}

QString Filter::cameraClause(PhotoDB &db) const {
  QStringList bits;

  if (!cameramake.isEmpty() || !cameramodel.isEmpty()) {
    // select on camera
    QSqlQuery q;
    if (cameramake.isEmpty()) {
      // select just on model
      q = db.query("select id from cameras where camera==:a",
		   cameramodel);
    } else if (cameramodel.isEmpty()) {
      // select just on make
      q = db.query("select id from cameras where make==:a",
		   cameramake);
    } else {
      // select on both
      q = db.query("select id from cameras where make==:a and camera==:b",
		   cameramake, cameramodel);
    }
    QStringList cameraids;
    while (q.next())
      cameraids << QString::number(q.value(0).toInt());
    if (cameraids.isEmpty())
      return "0>1";
    if (cameraids.size()==1)
      bits << "camera==" + cameraids.first();
    else
      bits << "camera in (" + cameraids.join(", ") + ")";
  }
  
  if (!cameralens.isEmpty()) {
    QSqlQuery q = db.query("select id from lenses where lens==:a", cameralens);
    if (q.next())
      bits << "lens==" + QString::number(q.value(0).toInt());
    else
      return "0>1";
  }

  if (bits.isEmpty())
    return "1>0";
  else
    return bits.join(" and ");      
}

QString Filter::dateRangeClause() const {
  return "capturedate>='" + startdate.toString("yyyy-MM-dd") + "T00:00:00'"
    + " and capturedate<='" + enddate.toString("yyyy-MM-dd") + "T23:59:59'";
}

QString Filter::fileLocationClause(PhotoDB &db) const {
  if (filelocation.isEmpty())
    return "0>1";
  QSet<int> folders;
  QSqlQuery q = db.query("select id from folders where pathname==:a",
			 filelocation);
  if (q.next())
    folders << q.value(0).toInt();
  q = db.query("select id from folders where pathname like :a",
	       filelocation + "/%");
  while (q.next())
    folders << q.value(0).toInt();
  if (folders.isEmpty())
    return "0>1";
  QStringList ss;
  for (int f: folders)
    ss << QString::number(f);
  return "folder in ( " + ss.join(", ") + " )";
}

QString Filter::tagsInterpretation(QStringList ss, class PhotoDB const &pdb) {
  Tags tags(pdb);
  QStringList res;
  for (auto s: ss) {
    s = s.simplified();
    QStringList alts;
    for (int t: tags.smartFindAll(s))
      alts << tags.smartName(t);
    if (alts.isEmpty())
      res << QString::fromUtf8("–");
    else if (alts.size()==1)
      res << alts.first();
    else
      res << "{" + alts.join(", ") + "}"; // This could be smarter:
    // If some of these alts actually occur in appliedtags and others do not,
    // only the occurring ones need to be shown. Right?
  }
  return res.join("\n");
}

QString Filter::tagsClause(PhotoDB &db) const {
  QStringList bits;
  Tags tdb(db);
  for (QString t: tags_) {
    QSet<int> tt = tdb.smartFindAll(t);
    if (tt.isEmpty())
      return "0>1";
    QStringList ss;
    for (int t: tt)
      ss << QString::number(t);
    bits << "versions.id in (select version from appliedtags where tag in ( "
      + ss.join(", ") + " ) )";
  }
  if (bits.isEmpty())
    return "1>0";
  return bits.join(" and ");
}

bool Filter::isTrivial() const {
  if (!(hascollection || hascolorlabels || hasstarrating || hasstatus
        || hascamera || hasdaterange || hasfilelocation))
    return true;
  //  return count()==Filter().count();
  return false;
}