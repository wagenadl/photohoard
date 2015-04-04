// AppliedTagList.h

#ifndef APPLIEDTAGLIST_H

#define APPLIEDTAGLIST_H

#include <QFrame>
#include "Tags.h"
#include "Selection.h"

class AppliedTagList: public QFrame {
  Q_OBJECT;
public:
  AppliedTagList(PhotoDB const &db, QWidget *parent=0);
  virtual ~AppliedTagList();
  virtual QSize sizeHint() const override;
  virtual QSize minimumSizeHint() const override;
  virtual int heightForWidth(int w) const override;
public slots:
  void setCurrent(quint64 vsn);
  void newSelection();
protected:
  virtual void resizeEvent(QResizeEvent *) override;
private:
  void rebuild();
  void relayout();
private slots:
  void removeTag(int);
  void applyTag(int);
  void editorAction();
  void clickBrowse();
private:
  QMap<int, class AppliedTagWidget *> widgets;
  class AppliedTagEditor *editor;
  class TagDialog *dialog;
  class QToolButton *browse;
  PhotoDB db;
  Tags tags;
  Selection selection;
  quint64 cur;
};

#endif
