// MetaViewer.h

#ifndef METAVIEWER_H

#define METAVIEWER_H

#include <QTextBrowser>
#include "Filter.h"

class MetaViewer: public QTextBrowser {
  Q_OBJECT;
public:
  MetaViewer(class SessionDB *, QWidget *parent=0);
  virtual ~MetaViewer() { }
public slots:
  void setVersion(quint64 version);
  void setImage(class Image16 const &, quint64 version);
  void handleClick(QUrl const &);
signals:
  void filterModified();
private:
  SessionDB *db;
};

#endif
