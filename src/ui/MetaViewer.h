// MetaViewer.h

#ifndef METAVIEWER_H

#define METAVIEWER_H

#include <QTextBrowser>
#include "PhotoDB.h"

class MetaViewer: public QTextBrowser {
  Q_OBJECT;
public:
  MetaViewer(PhotoDB *, QWidget *parent=0);
  virtual ~MetaViewer() { }
public slots:
  void setVersion(quint64 version);
  void handleClick(QUrl const &);
private:
  PhotoDB *db;
};

#endif
