// TagListWidget.cpp

#include "TagListWidget.h"
#include "PDebug.h"

TagListWidget::TagListWidget(PhotoDB *db, int parenttagid,
                             QWidget *parent):
  QListWidget(parent), db(db), parentid(parenttagid) {

  sw = TagDialog::ShowWhat::AllDefined;
  wid = 0;
  reload();
}

TagListWidget::~TagListWidget() {
}

QSize TagListWidget::sizeHint() const {
  return QSize(128, 192);
}

QList<int> TagListWidget::selectedTags() const {
  QList<QListWidgetItem *> sel = selectedItems();
  QList<int> tags;
  for (auto it: sel)
    if (it)
      tags << it->data(Qt::UserRole).toInt();
  return tags;
}

int TagListWidget::tagAt(int row) const {
  QListWidgetItem *it = item(row);
  if (it)
    return it->data(Qt::UserRole).toInt();
  else
    return 0;
}

void TagListWidget::load(int parenttagid) {
  parentid = parenttagid;
  reload();
}

void TagListWidget::setShown(TagDialog::ShowWhat s) {
  sw = s;
  reload();
}

void TagListWidget::reload() {
  clear();
  
  QStringList joins;
  QStringList wheres;
  wheres << (parentid==0 ? QString("parent is null")
             : QString("parent==%1").arg(parentid));
  switch (sw) {
  case TagDialog::ShowWhat::AllDefined:
    break;
  case TagDialog::ShowWhat::UsedInFilter:
    joins << "appliedtags on tags.id==appliedtags.tag";
    wheres << "appliedtags.version in (select version from filter)";
    break;
  case TagDialog::ShowWhat::UsedInLibrary:
    joins << "appliedtags on tags.id==appliedtags.tag";    
    break;
  case TagDialog::ShowWhat::UsedInSelectedVersions:
    joins << "appliedtags on tags.id==appliedtags.tag";    
    wheres << "appliedtags.version in (select version from selection)";
    break;
  }

  QString qq = "select distinct tags.id, tags.tag from tags";
  for (auto j: joins)
    qq += "inner join " + j;
  if (!wheres.isEmpty())
    qq += " where " + wheres.join("and");
  qq += " order by tags.tag";

  QList<int> ids;
  QStringList tags;
  { DBReadLock lock(db);
    QSqlQuery q = db->constQuery(qq);
    while (q.next()) {
      ids << q.value(0).toInt();
      tags << q.value(1).toString();
    }
  }
  for (int n=0; n<ids.size(); n++) {
    QListWidgetItem *it = new QListWidgetItem(tags[n]);
    it->setData(Qt::UserRole, QVariant(ids[n]));
    addItem(it);
  }
  recalculateWidth();
}

void TagListWidget::recalculateWidth() {
  int w1 = 50;
  for (int i=0; i<count(); i++) {
    auto it = item(i);
    int w = it->sizeHint().width();

    if (w>w1)
      w1 = w;
  }
  if (w1==wid)
    return;
  
  wid = w1 + 10;
  updateGeometry();
}
