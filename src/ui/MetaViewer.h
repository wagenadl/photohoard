// MetaViewer.h

#ifndef METAVIEWER_H

#define METAVIEWER_H

#include <QTextEdit>
#include "PhotoDB.h"

class MetaViewer: public QTextEdit {
  Q_OBJECT;
public:
  MetaViewer(PhotoDB const &, QWidget *parent=0);
  virtual ~MetaViewer() { }
public slots:
  void setVersion(quint64 version);
private:
  PhotoDB db;
  PhotoDB::VersionRecord vrec;
  PhotoDB::PhotoRecord prec;
};

#endif
