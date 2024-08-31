// ExportSettings.cpp

#include "ExportSettings.h"
#include "PhotoDB.h"
#include "PDebug.h"
#include <QFileInfo>
#include <QRegularExpression>

ExportSettings::ExportSettings() {
  fileFormat = FileFormat::JPEG;
  resolutionMode = ResolutionMode::Full;
  maxdim = 1000;
  scalePercent = 50;
  jpegQuality = 95;
  namingScheme = NamingScheme::Original;
}

bool ExportSettings::isValid() const {
  if (destination.isEmpty())
    return false;
  QFileInfo fi(destination);
  return fi.isWritable();
}

void ExportSettings::loadFromDB(PhotoDB const *db) {
  ASSERT(db);
  DBReadLock lock(db);
  QSqlQuery q = db->constQuery("select k, v from exportsettings");
  while (q.next()) {
    QString k = q.value(0).toString();
    QVariant v = q.value(1);
    if (k=="ff")
      fileFormat = FileFormat(v.toInt());
    else if (k=="rm")
      resolutionMode = ResolutionMode(v.toInt());
    else if (k=="md")
      maxdim = v.toInt();
    else if (k=="sp")
      scalePercent = v.toInt();
    else if (k=="jq")
      jpegQuality = v.toInt();
    else if (k=="ns")
      namingScheme = NamingScheme(v.toInt());
    else if (k=="dst")
      destination = v.toString();
    else
      CRASH("Unknown key in exportsettings");
  }
}

void ExportSettings::saveToDB(PhotoDB *db) {
  Transaction t(db);
  pDebug() << "Export stdb";
  db->query("delete from exportsettings");
  QString q = "insert into exportsettings values (:a, :b)";
  db->query(q, "ff", int(fileFormat));
  db->query(q, "rm", int(resolutionMode));
  db->query(q, "md", maxdim);
  db->query(q, "sp", scalePercent);
  db->query(q, "jq", jpegQuality);
  db->query(q, "ns", int(namingScheme));
  db->query(q, "dst", destination);
  t.commit();
}

QString ExportSettings::extension() const {
  switch (fileFormat) {
  case FileFormat::JPEG:
    return "jpg";
  case FileFormat::PNG:
    return "png";
  case FileFormat::TIFF:
    return "tif";
  }
  return QString();
}

QString ExportSettings::exportFilename(PhotoDB *db, quint64 vsn) const {
  PhotoDB::PhotoRecord prec = db->photoRecord(db->photoFromVersion(vsn));
  QString ofn;
  switch (namingScheme) {
  case ExportSettings::NamingScheme::Original:
    ofn = prec.filename;
    if (ofn.contains("."))
      ofn = ofn.left(ofn.lastIndexOf("."));
    break;
  case ExportSettings::NamingScheme::DateTime:
    ofn = prec.capturedate.toString("yyMMdd-hhmmss");
    break;
  case ExportSettings::NamingScheme::DateTimeDSC: {
    ofn = prec.capturedate.toString("yyMMdd-hhmmss");
    QRegularExpression dd("\\d+");
    QStringList numbits;
    int idx = 0;
    while (true) {
      QRegularExpressionMatch m = dd.match(prec.filename, idx);
      if (m.hasMatch()) {
        numbits << m.captured();
        idx = m.capturedEnd();
      } else {
        break;
      }
    }
    if (numbits.size() == 1)
      ofn += "-" + numbits[0];
    else
      ofn += "_" + QString::number(vsn);
  } break;
  }
  ofn = destination + "/" + ofn + "." + extension();
  return ofn;
}
