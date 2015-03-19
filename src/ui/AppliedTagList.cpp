// AppliedTagList.cpp

#include "AppliedTagList.h"
#include "AppliedTagWidget.h"
#include "AppliedTagEditor.h"

AppliedTagList::AppliedTagList(PhotoDB const &db, QWidget *parent):
  QFrame(parent),
  db(db), tags(db), selection(db) {
  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
  editor = new AppliedTagEditor(db, this);
  cur = 0;
}

AppliedTagList::~AppliedTagList() {
}

QSize AppliedTagList::sizeHint() const {
  return childrenRect().size();    
}

QSize AppliedTagList::minimumSizeHint() const {
  return QSize(0, sizeHint().height()); // hmmm.
}

void AppliedTagList::setCurrent(quint64 vsn) {
  cur = vsn;
  rebuild();
}

void AppliedTagList::newSelection() {
  rebuild();
}

void AppliedTagList::resizeEvent(QResizeEvent *) {
  relayout();
}

void AppliedTagList::rebuild() {
  QSet<quint64> sel = selection.current();
  QSet<int> tagsInAny;
  QSet<int> tagsInCur = tags.applied(cur);
  QSet<int> tagsInAll = tagsInCur;
  for (auto id: sel) {
    QSet<int> tt = tags.applied(id);
    tagsInAny |= tt;
    tagsInAll &= tt;
  }

  // Remove tags we no longer have
  for (auto t: widgets.keys()) {
    if (!tagsInAll.contains(t)) {
      widgets[t]->deleteLater();
      widgets.remove(t);
    }
  }

  for (auto t: tagsInAny) {
    if (!widgets.contains(t)) {
      widgets[t] = new AppliedTagWidget(t, tags.smartName(t),
					this);
      connect(widgets[t], SIGNAL(removeClicked(int)),
	      SLOT(removeTag(int)));
      connect(widgets[t], SIGNAL(addClicked(int)),
	      SLOT(applyTag(int)));
    }
    widgets[t]->setInclusion(tagsInCur.contains(t),
			     tagsInAll.contains(t));
  }
  editor->reset();
  relayout();
}

void AppliedTagList::removeTag(int id) {
  QSet<quint64> sel = selection.current();
  for (int vsn: sel)
    tags.remove(vsn, id);
  rebuild();
}

void AppliedTagList::applyTag(int id) {
  QSet<quint64> sel = selection.current();
  for (int vsn: sel)
    tags.apply(vsn, id);
  rebuild();
}
  
void AppliedTagList::relayout() {
  int w = width();
  int x = 0;
  int y = 0;
  int lh = 0;
  for (auto ptr: widgets) {
    int w1 = ptr->width();
    if (x+w1>w && x!=0) {
      y += lh;
      x = 0;
      lh = 0;
    }
    ptr->move(x, y);
    int h = ptr->height();
    if (h>lh)
      lh = h;
  }

  y += lh;
  x = 0;
  editor->move(x, y);
}

int AppliedTagList::heightForWidth(int w) const {
  int x = 0;
  int y = 0;
  int lh = 0;
  for (auto ptr: widgets) {
    int w1 = ptr->width();
    if (x+w1>w && x!=0) {
      y += lh;
      x = 0;
      lh = 0;
    }
    int h = ptr->height();
    if (h>lh)
      lh = h;
  }

  y += lh;
  x = 0;
  lh = editor->height();
  
  return y + lh;
}
