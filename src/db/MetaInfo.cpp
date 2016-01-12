// MetaInfo.cpp

#include "MetaInfo.h"
#include "PhotoDB.h"
#include "Exif.h"
#include <QStringList>
#include "PDebug.h"

QString MetaInfo::ratio(int w, int h) {
  constexpr double marg = 2e-3;
  double r = double(w)/double(h);
  if (fabs(r-1)<marg)
    return "1:1";
  else if (fabs(r-4/3.)<marg)
    return "4:3";
  else if (fabs(r-3/2.)<marg)
    return "3:2";
  else if (fabs(r-16/9.)<marg)
    return "16:9";
  else if (fabs(r-3/4.)<marg)
    return "3:4";
  else if (fabs(r-2/3.)<marg)
    return "2:3";
  else
    return "";
}

QString MetaInfo::mpix(int w, int h) {
  double mp = w*h/1e6;
  if (mp<3)
    return QString::number(mp, 'f', 1);
  else
    return QString::number(int(round(mp)));
}


MetaInfo::MetaInfo(PhotoDB *db, quint64 version) {
  if (version==0) {
    txt = "";
    return;
  }

  PhotoDB::VersionRecord vrec = db->versionRecord(version);
  PhotoDB::PhotoRecord prec = db->photoRecord(vrec.photo);

  // Add filename
  txt = QString("<b>%1</b><br>").arg(prec.filename);
  QString folder = db->folder(prec.folderid);
  folder = folder.replace("/", QString("/") + QChar(0x200b));
  txt += QString("<i>%1</i><br>").arg(folder);

  // Add capture date
  txt += "<b>" + prec.capturedate.toString("ddd") + " ";
  QString yr = prec.capturedate.toString("yyyy");
  QString md = prec.capturedate.toString("MMdd");
  txt += "<a href=\"date:" + yr + md + "\">"
    + prec.capturedate.toString("MMM d") + "</a>";
  txt += ", ";
  txt += "<a href=\"year:" + yr + "\">"
    + prec.capturedate.toString("yyyy") + "</a>";
  txt += ", ";
  txt += prec.capturedate.toString("hh:mm:ss") + "</b><br>";

  // Add exposure info
  QStringList bits;
  if (prec.exposetime_s>0)
    bits << (prec.exposetime_s > .125
	     ? QString::fromUtf8("%1”").arg(prec.exposetime_s)
	     : QString::fromUtf8("1/%1”").arg(round(1/prec.exposetime_s)));
  if (prec.fnumber>0)
    bits << QString("f/%1").arg(prec.fnumber);
  if (prec.iso>0)
    bits << QString("ISO %1").arg(prec.iso);
  if (!bits.isEmpty())
    txt += bits.join(" &nbsp; ") + "<br>";

  // Add camera and lens info
  QString camlens = "";
  if (prec.cameraid) {
    QString cam = db->camera(prec.cameraid);
    camlens = ("<a href=\"camera:%1\">" + cam + "</a>").arg(prec.cameraid);
  }
  QString lens = "";
  if (prec.lensid) {
    if (!camlens.isEmpty())
      camlens += ", ";
    lens = db->lensAlias(prec.lensid);
    camlens += ("<a href=\"lens:%1\">" + lens + "</a>").arg(prec.lensid);
  }
  if (camlens.isEmpty()) 
    camlens = "<i>unknown camera</i>";
  if (prec.focallength_mm>0) {
    // Add focal lens to description, unless lens is fixed length and
    // already contains focal length in its name or alias
    QString len = QString("%1").arg(prec.focallength_mm);
    if (!(lens.contains(len) && !lens.contains(len + "-") 
	  && !lens.contains("-" + len)))
      camlens += " at " + len + " mm";
  }
  txt += QString("%1<br>").arg(camlens);

  // Add size of crop
  

  // Add size of original
  PSize photosize = Exif::fixOrientation(prec.filesize, vrec.orient);
  QString rat = ratio(photosize.width(), photosize.height());
  if (!rat.isEmpty())
    rat = ", " + rat;
  txt += QString("%1 x %2 (%3 MPix%4)").arg(photosize.width())
    .arg(photosize.height())
    .arg(mpix(photosize.width(), photosize.height()))
    .arg(rat);
}  
  
