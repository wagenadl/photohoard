// Filter.cpp

#include "Filter.h"
#include "SessionDB.h"
#include "Tags.h"
#include "PDebug.h"

Filter::Filter(SessionDB *db): db(db) {
  qRegisterMetaType<Filter>("Filter");
  reset();
}

void Filter::reset() {
  hascollection = false;
  collection_ = "";
  hascolorlabels = false;
  colorlabels.clear();
  hasstarrating = false;
  minstars = 0;
  maxstars = 5;
  hasstatus = true;
  statusaccepted = statusunset = statusnew = true;
  statusrejected = false;
  hascamera = false;
  cameramake = cameramodel = cameralens = "";
  hasdaterange = false;
  startdate = QDate(1980, 1, 1);
  enddate = QDate(QDate::currentDate().year(), 12, 31);
  hasfilelocation = false;
  filelocation = "";
  hastags = false;
  tags_.clear();
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

void Filter::setStatus(bool accepted, bool unset, bool rejected, bool newi) {
  hasstatus = true;
  statusaccepted = accepted;
  statusunset = unset;
  statusrejected = rejected;
  statusnew = newi;
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

int Filter::count() const {
  ASSERT(db);
  return db->simpleQuery("select count(*) from versions "
			+ joinClause()
			+ " where " + whereClause()).toInt();
}

QString Filter::joinClause() const {
  QStringList joins;
  //  if (hasdaterange || hasfilelocation || hascamera)
  joins << "inner join photos on versions.photo==photos.id";
  return joins.join(" ");
}

QString Filter::whereClause() const {
  ASSERT(db);
  QStringList clauses;
  if (hascollection)
    clauses << collectionClause();
  if (hascolorlabels)
    clauses << colorLabelClause();
  if (hasstarrating)
    clauses << starRatingClause();
  if (hasstatus)
    clauses << statusClause();
  pDebug() << "filter: whereclause hascamera" << hascamera;
  if (hascamera)
    clauses << cameraClause();
  if (hasdaterange)
    clauses << dateRangeClause();
  if (hasfilelocation)
    clauses << fileLocationClause();
  if (hastags)
    clauses << tagsClause();
  if (clauses.isEmpty())
    return "1>0";
  else
    return clauses.join(" and ");
}

QString Filter::collectionClause() const {
  QSqlQuery q = db->query("select id from tags where tag==:a", "Collections");
  if (!q.next())
    return collection_.isEmpty() ? "1>0" : "0>1";
  int colparent = q.value(0).toInt();
  if (collection_.isEmpty()) {
    return "versions.id not in ( select version from appliedtags where tag in "
      "( select id from tags where parent==" + QString::number(colparent)
      +") )";
  } else {
    q = db->query("select id from tags where tag==:a and parent==:b",
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
  if (statusaccepted || statusunset || statusrejected || statusnew) {
    QStringList vv;
    if (statusaccepted)
      vv << QString::number(int(PhotoDB::AcceptReject::Accept));
    if (statusunset)
      vv << QString::number(int(PhotoDB::AcceptReject::Undecided));
    if (statusrejected)
      vv << QString::number(int(PhotoDB::AcceptReject::Reject));
    if (statusnew)
      vv << QString::number(int(PhotoDB::AcceptReject::NewImport));
    return "acceptreject in ( " + vv.join(", ") + " )";
  } else {
    return "0>1";
  }
}

QString Filter::cameraClause() const {
  QStringList bits;

  pDebug() << "cameraClause" << cameramake.isEmpty() << cameramodel.isEmpty()
	   << cameramake << cameramodel;
  if (!cameramake.isEmpty() || !cameramodel.isEmpty()) {
    // select on camera
    QSqlQuery q;
    if (cameramake.isEmpty()) {
      // select just on model
      q = db->query("select id from cameras where camera==:a",
		   cameramodel);
    } else if (cameramodel.isEmpty()) {
      // select just on make
      q = db->query("select id from cameras where make==:a",
		   cameramake);
    } else {
      // select on both
      q = db->query("select id from cameras where make==:a and camera==:b",
		   cameramake, cameramodel);
    }
    QStringList cameraids;
    while (q.next())
      cameraids << QString::number(q.value(0).toInt());
    pDebug() << "cameraids" << cameraids.size();
    if (cameraids.isEmpty())
      return "0>1";
    if (cameraids.size()==1)
      bits << "camera==" + cameraids.first();
    else
      bits << "camera in (" + cameraids.join(", ") + ")";
  }
  
  if (!cameralens.isEmpty()) {
    QSqlQuery q = db->query("select id from lenses where lens==:a", cameralens);
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

QString Filter::fileLocationClause() const {
  if (filelocation.isEmpty())
    return "0>1";
  QSet<int> folders;
  QSqlQuery q = db->query("select id from folders where pathname==:a",
			 filelocation);
  if (q.next())
    folders << q.value(0).toInt();
  q = db->query("select id from folders where pathname like :a",
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

QString Filter::tagsInterpretation(QStringList ss) {
  Tags tags(db);
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

QString Filter::tagsClause() const {
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

void Filter::saveToDb() const {
  ASSERT(db);
  Transaction t(db);
  db->query("delete from filtersettings");
  QString q = "insert into filtersettings values (:a, :b)";
  db->query(q, "hascol", hascollection);
  db->query(q, "col", collection_);
  db->query(q, "hascl", hascolorlabels);
  for (auto c: colorlabels)
    db->query(q, "cl", int(c));
  db->query(q, "hassr", hasstarrating);
  db->query(q, "minsr", minstars);
  db->query(q, "maxsr", maxstars);
  db->query(q, "hasst", hasstatus);
  db->query(q, "s_acc", statusaccepted);
  db->query(q, "s_rej", statusrejected);
  db->query(q, "s_uns", statusunset);
  db->query(q, "s_new", statusnew);
  db->query(q, "hascam", hascamera);
  db->query(q, "cammake", cameramake);
  db->query(q, "cammodel", cameramodel);
  db->query(q, "camlens", cameralens);
  db->query(q, "hasdr", hasdaterange);
  db->query(q, "startdate", startdate);
  db->query(q, "enddate", enddate);
  db->query(q, "hasfl", hasfilelocation);
  db->query(q, "fl", filelocation);
  db->query(q, "hastags", hastags);
  for (auto s: tags_)
    db->query(q, "tag", s);
  t.commit();
}

void Filter::loadFromDb() {
  ASSERT(db);
  reset();
  QSqlQuery q = db->query("select k, v from filtersettings");
  while (q.next()) {
    QString k = q.value(0).toString();
    QVariant v = q.value(1);
    if (k=="hascol")
      hascollection = v.toBool();
    else if (k=="col")
      collection_ = v.toString();
    else if (k=="hascl")
      hascolorlabels = v.toBool();
    else if (k=="cl")
      colorlabels.insert(PhotoDB::ColorLabel(v.toInt()));
    else if (k=="hassr")
      hasstarrating = v.toBool();
    else if (k=="minsr")
      minstars = v.toInt();
    else if (k=="maxsr")
      maxstars = v.toInt();
    else if (k=="hasst")
      hasstatus = v.toBool();
    else if (k=="s_acc")
      statusaccepted = v.toBool();
    else if (k=="s_uns")
      statusunset = v.toBool();
    else if (k=="s_rej")
      statusrejected = v.toBool();
    else if (k=="s_new")
      statusnew = v.toBool();
    else if (k=="hascam")
      hascamera = v.toBool();
    else if (k=="cammake")
      cameramake = v.toString();
    else if (k=="cammodel")
      cameramodel = v.toString();
    else if (k=="camlens")
      cameralens = v.toString();
    else if (k=="hasdr")
      hasdaterange = v.toBool();
    else if (k=="startdate")
      startdate = v.toDate();
    else if (k=="enddate")
      enddate = v.toDate();
    else if (k=="hasfl")
      hasfilelocation = v.toBool();
    else if (k=="fl")
      filelocation = v.toString();
    else if (k=="hastags")
      hastags = v.toBool();
    else if (k=="tag")
      tags_.append(v.toString());
    else
      CRASH("Filter: unknown filter key: " + k);
  }
}
