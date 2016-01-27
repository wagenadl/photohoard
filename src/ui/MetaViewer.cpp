// MetaViewer.cpp

#include "MetaViewer.h"
#include "MetaInfo.h"
#include "PDebug.h"
#include <QUrl>

MetaViewer::MetaViewer(PhotoDB *db, QWidget *parent):
  QTextBrowser(parent), db(db) {
  setReadOnly(true);
  setFocusPolicy(Qt::NoFocus);
  setOpenLinks(false);
  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
  document()->setDefaultStyleSheet("a { color: 'white'; }\n");
  connect(this, SIGNAL(anchorClicked(QUrl const &)),
	  SLOT(handleClick(QUrl const &)));
}

void MetaViewer::setVersion(quint64 version) {
  if (version==0) {
    document()->setHtml("");
    setEnabled(false);
    return;
  }
  setEnabled(true);
  document()->setHtml(MetaInfo(db, version).html());
}

void MetaViewer::handleClick(QUrl const &url) {
  COMPLAIN("MetaViewer: click: not yet implemented");
}
