// MetaViewer.cpp

#include "MetaViewer.h"
#include "MetaInfo.h"
#include "PDebug.h"
#include <QUrl>
#include "Filter.h"
#include "SessionDB.h"

MetaViewer::MetaViewer(SessionDB *db, QWidget *parent):
  QTextBrowser(parent), db(db) {
  setReadOnly(true);
  setFocusPolicy(Qt::NoFocus);
  setOpenLinks(false);
  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
  document()->setDefaultStyleSheet("a { color: '#88ccff'; text-decoration: none }\n");
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
  pDebug () << "handleclick" << url;
  Filter flt(db);
  flt.loadFromDb();
  if (MetaInfo::modifyFilterWithLink(flt, url)) {
    pDebug() << "Filter now" << flt.whereClause();
    flt.saveToDb();
    emit filterModified();
  }
}

void MetaViewer::setImage(class Image16 const &, quint64 version) {
  setVersion(version);
}
