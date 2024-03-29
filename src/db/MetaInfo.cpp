// MetaInfo.cpp

#include "MetaInfo.h"
#include "PhotoDB.h"
#include "Exif.h"
#include <QStringList>
#include "PDebug.h"
#include "Adjustments.h"
#include <math.h>
#include <QUrl>
#include "Filter.h"

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
      return bits[1] + ":" + bits[0];
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
    else if (fabs(r-5/3.5)<marg)
      return QString::fromUtf8("5:3½");
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
    if (rat=="1:1+") {
      return "1:1-";
    } else if (rat.indexOf(":")<0) {
      return "/" + rat;
    } else {
      QStringList bits = rat.split(":");
      QString sfx = "";
      if (bits[1].endsWith("+")) {
        bits[1] = bits[1].left(bits[1].length()-1);
        sfx = "-";
      } else if (bits[1].endsWith("-")) {
        bits[1] = bits[1].left(bits[1].length()-1);
        sfx = "-";
      } 
      return bits[1] + ":" + bits[0] + sfx;
    }
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
    else if (r<inbetween(4/3.,11/8.5))
      return "4:3+";
    else if (r<11/8.5)
      return "Letter-";
    else if (r<inbetween(11/8.5, 297/210.))
      return "Letter+";
    else if (r<297/210.)
      return "A4-";
    else if (r<inbetween(297/210., 5/3.5))
      return "A4+";
    else if (r<5/3.5)
      return QString::fromUtf8("5:3½-");
    else if (r<inbetween(5/3.5, 3/2.))
      return QString::fromUtf8("5:3½+");
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
  QString r = easyRatio(s.width(), s.height());
  r.replace("-", QString::fromUtf8("⁻"));
  r.replace("+", QString::fromUtf8("⁺"));
  return r;
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
  Adjustments sliders = Adjustments::fromDB(version, *db);

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
    camlens = QString("<a href=\"camera:%1:%2:%3\">%4</a>")
      .arg(prec.cameraid)
      .arg(db->make(prec.cameraid))
      .arg(db->model(prec.cameraid))
      .arg(db->cameraAlias(prec.cameraid));
  }
  QString lens = "";
  if (prec.lensid) {
    lens = db->lensAlias(prec.lensid);
    if (!camlens.isEmpty())
      camlens += ", ";
    camlens += QString("<a href=\"lens:%1:%2\">%3</a>")
      .arg(prec.lensid)
      .arg(db->lens(prec.lensid))
      .arg(lens);
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
  txt += QString("%1 x %2").arg(cropsize.width()).arg(cropsize.height());
  txt += QString(" (%1 MPix %2").arg(mpix(cropsize)).arg(easyRatio(cropsize));

  // Add size of original
  PSize photosize = Exif::fixOrientation(prec.filesize, vrec.orient);
  if (photosize != cropsize)
    txt += QString("; cropped from %1 x %2").arg(photosize.width())
      .arg(photosize.height());

  txt += ")";
}  
  
bool MetaInfo::modifyFilterWithLink(Filter &filter, QUrl const &url) {
  QString str = url.toString();
  QStringList bits = str.split(":");
  if (bits.isEmpty())
    return false;
  QString key = bits.takeFirst();
  if (key=="camera") {
    if (filter.hasCamera()) {
      if (filter.cameraMake()==bits[1]
	  && filter.cameraModel()==bits[2]) 
	filter.unsetCamera();
      else
	filter.setCamera(bits[1], bits[2], filter.cameraLens());
    } else {
      filter.setCamera(bits[1], bits[2], "");
    }
    return true;
  } else if (key=="lens") {
    if (filter.hasCamera()) {
      if (filter.cameraLens()==bits[1])
	filter.unsetCamera();
      else
	filter.setCamera(filter.cameraMake(), filter.cameraModel(),
			  bits[1]);
    } else {
      filter.setCamera("", "", bits[1]);
    }
    return true;
  } else if (key=="date") {
    QDate dt = QDate::fromString(bits[0], "yyyyMMdd");
    if (filter.hasDateRange()
	&& filter.startDate()==dt
	&& filter.endDate()==dt)
      filter.unsetDateRange();
    else
      filter.setDateRange(dt, dt);
    return true;
  } else if (key=="year") {
    QDate dt0 = QDate::fromString(bits[0]+"0101", "yyyyMMdd");
    QDate dt1 = QDate::fromString(bits[0]+"1231", "yyyyMMdd");
    if (filter.hasDateRange()
	&& filter.startDate()==dt0
	&& filter.endDate()==dt1)
      filter.unsetDateRange();
    else
      filter.setDateRange(dt0, dt1);
    return true;
  }
  return false;
}
