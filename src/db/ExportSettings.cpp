// ExportSettings.cpp

#include "ExportSettings.h"
#include "PhotoDB.h"
#include "PDebug.h"
#include <QFileInfo>

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
