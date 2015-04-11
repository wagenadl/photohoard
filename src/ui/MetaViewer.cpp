// MetaViewer.cpp

#include "MetaViewer.h"
#include "MetaInfo.h"

MetaViewer::MetaViewer(PhotoDB *db, QWidget *parent):
  QTextEdit(parent), db(db) {
  setReadOnly(true);
  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
}

void MetaViewer::setVersion(quint64 version) {
  if (version==0) {
    setHtml("");
    setEnabled(false);
    return;
  }
  setEnabled(true);
  setHtml(MetaInfo(db, version).html());
}

