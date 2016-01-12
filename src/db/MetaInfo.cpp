// MetaInfo.cpp

#include "MetaInfo.h"
#include "PhotoDB.h"
#include "Exif.h"
#include <QStringList>
#include "PDebug.h"
#include "Sliders.h"

inline double inbetween(double a, double b) {
  return sqrt(a*b);
}

QString MetaInfo::ratio(int w, int h) {
  if (h<=0 || w<=0)
    return "";
  double r = double(w)/double(h);
  if (r<1) {
    QString rat = ratio(h, w);
    if (rat.isEmpty()) {
      return "";
    } else if (rat.indexOf(":")<0) {
      return "/" + rat;
    } else {
      QStringList bits = rat.split(":");
      return rat[1] + ":" + rat[0];
    }
  } else {
    double marg = 1./w + 1./h;
    if (fabs(r-1)<marg)
      return "1:1";
    else if (fabs(r-5/4.)<marg)
      return "5:4";
    else if (fabs(r-4/3.)<marg)
      return "4:3";
    else if (fabs(r-11/8.5)<marg)
      return "Letter";
    else if (fabs(r-297/210.)<marg)
      return "A4";
    else if (fabs(r-3/2.)<marg)
      return "3:2";
    else if (fabs(r-16/10.)<marg)
      return "16:10";
    else if (fabs(r-1.618034)<marg)
      return "Golden";
    else if (fabs(r-16/9.)<marg)
      return "16:9";
    else
      return "";
  }
}

QString MetaInfo::ratio(PSize s) {
  return ratio(s.width(), s.height());
}

QString MetaInfo::easyRatio(int w, int h) {
  if (w<=0 || h<=0)
    return "";

  QString r0 = ratio(w, h);
  if (!r0.isEmpty())
    return r0;

  if (w<h) {
    QString rat = easyRatio(h, w);
    if (rat=="1:1+") 
      return "1:1-";
    else
      return rat;
  } else {
    double r = double(w)/double(h);
    if (r<inbetween(1, 5/4.))
      return "1:1+";
    else if (r<5/4.)
      return "5:4-";
    else if (r<inbetween(5/4., 4/3.))
      return "5:4+";
    else if (r<4/3.)
      return "4:3-";
    else if (r<inbetween(4/3,11/8.5))
      return "4:3+";
    else if (r<11/8.5)
      return "Letter-";
    else if (r<inbetween(11/8.5, 297/210.))
      return "Letter+";
    else if (r<297/210.)
      return "A4-";
    else if (r<inbetween(297/210., 3/2.))
      return "A4+";
    else if (r<3/2.)
      return "3:2-";
    else if (r<inbetween(3/2., 16/10.))
      return "3:2+";
    else if (r<16/10.)
      return "16:10-";
    else if (r<inbetween(16/10., 1.618034))
      return "16:10+";
    else if (r<1.618034)
      return "Golden-";
    else if (r<inbetween(1.618034, 16/9.))
      return "Golden+";
    else if (r<16/9.)
      return "16:9-";
    else
      return "16:9+";
  }
}

QString MetaInfo::easyRatio(PSize s) {
  return easyRatio(s.width(), s.height());
}

QString MetaInfo::mpix(int w, int h) {
  double mp = w*h/1e6;
  if (mp<3)
    return QString::number(mp, 'f', 1);
  else
    return QString::number(int(round(mp)));
}

QString MetaInfo::mpix(PSize s) {
  return mpix(s.width(), s.height());
}

MetaInfo::MetaInfo(PhotoDB *db, quint64 version) {
  if (version==0) {
    txt = "";
    return;
  }

  PhotoDB::VersionRecord vrec = db->versionRecord(version);
  PhotoDB::PhotoRecord prec = db->photoRecord(vrec.photo);
  Sliders sliders = Sliders::fromDB(version, *db);

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
  PSize cropsize = sliders.cropSize(prec.filesize, vrec.orient);
  qDebug() << prec.filesize << int(vrec.orient) << cropsize << version;
  txt += QString("%1 x %2").arg(cropsize.width()).arg(cropsize.height());
  txt += QString(" (%1 MPix %2").arg(mpix(cropsize)).arg(easyRatio(cropsize));

  // Add size of original
  PSize photosize = Exif::fixOrientation(prec.filesize, vrec.orient);
  if (photosize != cropsize)
    txt += QString("; cropped from %1 x %2").arg(photosize.width())
      .arg(photosize.height());

  txt += ")";
}  
  
