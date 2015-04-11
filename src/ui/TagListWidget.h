// TagListWidget.h

#ifndef TAGLISTWIDGET_H

#define TAGLISTWIDGET_H

#include <QListWidget>
#include "PhotoDB.h"

#include "TagDialog.h"

class TagListWidget: public QListWidget {
  Q_OBJECT;
public:
  TagListWidget(PhotoDB *db, int parenttagid, QWidget *parent=0);
  virtual ~TagListWidget();
  virtual QSize sizeHint() const;
  bool isEmpty() const { return count()==0; }
public:
  QList<int> selectedTags() const;
  int tagAt(int row) const;
public slots:
  void reload();
  void load(int parenttagid);
  void setShown(TagDialog::ShowWhat);
private:
  void recalculateWidth();
private:
  PhotoDB *db;
  int parentid;
  TagDialog::ShowWhat sw;
  int wid;
};

#endif
