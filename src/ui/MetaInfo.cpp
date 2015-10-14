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


  txt = QString("<b>%1</b><br>").arg(prec.filename);
  QString folder = db->folder(prec.folderid);
  folder = folder.replace("/", QString("/") + QChar(0x200b));
  txt += QString("<i>%1</i><br>").arg(folder);
  txt += "<b>"
    + prec.capturedate.toString("ddd MMM d, yyyy, hh:mm:ss") + "</b><br>";
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
  QString cam = prec.cameraid ? db->camera(prec.cameraid)
    : QString("<i>unknown camera</i>");
  QString lens = prec.lensid ? db->lens(prec.lensid)
    : QString("<i>unknown lens</i>");
  QString camlens = (prec.cameraid && prec.lensid) ? cam + ", " + lens : cam;
  if (prec.focallength_mm>0) {
    QString len = QString("%1").arg(prec.focallength_mm);
    if (!(camlens.contains(len) && !camlens.contains(len+"-") 
	  && !camlens.contains("-"+len)))
      camlens += " at " + len + " mm";
  }
  txt += QString("%1<br>").arg(camlens);
  PSize photosize = Exif::fixOrientation(prec.filesize, vrec.orient);
  QString rat = ratio(photosize.width(), photosize.height());
  if (!rat.isEmpty())
    rat = ", " + rat;
  txt += QString("%1 x %2 (%3 MPix%4)").arg(photosize.width())
    .arg(photosize.height())
    .arg(mpix(photosize.width(), photosize.height()))
    .arg(rat);
}  
  
