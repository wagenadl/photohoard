// MetaViewer.cpp

#include "MetaViewer.h"
#include "Exif.h"

MetaViewer::MetaViewer(PhotoDB const &db, QWidget *parent):
  QTextEdit(parent), db(db) {
  setReadOnly(true);
  vrec.id = 0;
  prec.id = 0;
}

void MetaViewer::setVersion(quint64 version) {
  vrec = db.versionRecord(version);
  prec = db.photoRecord(vrec.photo);

  QString html = QString("<b>%1</b><br>").arg(prec.filename);
  QString folder = db.folder(prec.folderid);
  folder = folder.replace("/", QString("/") + QChar(0x200b));
  html += QString("<i>%1</i><br>").arg(folder);
  html += "<b>"
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
    html += bits.join(" &nbsp; ") + "<br>";
  QString cam = prec.cameraid ? db.camera(prec.cameraid)
    : QString("<i>unknown camera</i>");
  QString lens = prec.lensid ? db.lens(prec.lensid)
    : QString("<i>unknown lens</i>");
  QString camlens = (prec.cameraid && prec.lensid) ? cam + ", " + lens : cam;
  if (prec.focallength_mm>0) {
    QString len = QString("%1").arg(prec.focallength_mm);
    if (!(camlens.contains(len) && !camlens.contains(len+"-") 
	  && !camlens.contains("-"+len)))
      camlens += " at " + len + " mm";
  }
  html += QString("%1<br>").arg(camlens);
  PSize photosize = Exif::fixOrientation(prec.filesize, prec.orient);
  html += QString("%1 x %2 (%3 MPix)").arg(photosize.width())
    .arg(photosize.height())
    .arg(round(photosize.width()*photosize.height()/1e6));
  setHtml(html);
}

