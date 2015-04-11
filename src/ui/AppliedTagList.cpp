// AppliedTagList.cpp

#include "AppliedTagList.h"
#include "AppliedTagWidget.h"
#include "AppliedTagEditor.h"
#include "PDebug.h"
#include <QToolButton>
#include "TagDialog.h"

AppliedTagList::AppliedTagList(PhotoDB *db, QWidget *parent):
  QFrame(parent),
  db(db), tags(db), selection(db) {
  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

  editor = new AppliedTagEditor(db, this);
  connect(editor, SIGNAL(returnPressed()),
	  SLOT(editorAction()));

  dialog = new TagDialog(db);
  connect(dialog, SIGNAL(apply(int)), SLOT(applyTag(int)));
  connect(dialog, SIGNAL(unapply(int)), SLOT(removeTag(int)));
  browse = new QToolButton(this);
  browse->setText(QString::fromUtf8("…"));
  connect(browse, SIGNAL(clicked()), SLOT(clickBrowse()));
  
  cur = 0;
  setAutoFillBackground(true);
  setContentsMargins(2,2,2,2);
}

AppliedTagList::~AppliedTagList() {
}

QSize AppliedTagList::sizeHint() const {
  QMargins m = contentsMargins();
  QRect cc = editor->geometry().adjusted(0, 0, 2 + browse->width(), 0);
  for (auto w: widgets)
    cc |= w->rect();
  return cc.size() + QSize(m.left() + m.right(),
                           m.top() + m.bottom());    
}

QSize AppliedTagList::minimumSizeHint() const {
  return QSize(50, sizeHint().height()); // hmmm.
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
  setEnabled(cur>0);
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
    if (!tagsInAny.contains(t)) {
      widgets[t]->deleteLater();
      widgets.remove(t);
    }
  }

  for (auto t: tagsInAny) {
    if (!widgets.contains(t)) {
      widgets[t] = new AppliedTagWidget(t, tags.smartName(t), this);
      connect(widgets[t], SIGNAL(removeClicked(int)),
	      SLOT(removeTag(int)));
      connect(widgets[t], SIGNAL(addClicked(int)),
	      SLOT(applyTag(int)));
      widgets[t]->show();
    }
    widgets[t]->setInclusion(tagsInCur.contains(t),
			     tagsInAll.contains(t));
  }
  editor->reset();
  relayout();
}

void AppliedTagList::removeTag(int id) {
  pDebug() << "ATL:remove" << id;
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
  QRect r = contentsRect();
  int w = r.width();
  int x = r.left();
  int y = r.top();
  int lh = 0;
  for (auto ptr: widgets) {
    ptr->resize(ptr->sizeHint());
    int w1 = ptr->width();
    if (x+w1>w && x>r.left()) {
      y += lh + 1;
      x = r.left();
      lh = 0;
    }
    ptr->move(x, y);
    int h = ptr->height();
    if (h>lh)
      lh = h;
    x += w1 + 8;
  }

  y += lh + 1;
  x = r.left();
  editor->move(x, y);
  browse->resize(browse->sizeHint().width(), editor->height());
  browse->move(r.right()-browse->width(), r.bottom()-browse->height());
  updateGeometry();
}

int AppliedTagList::heightForWidth(int w) const {
  QMargins m = contentsMargins();
  w -= m.left() + m.right();
  int x = 0;
  int y = m.top() + m.bottom();
  int lh = 0;
  for (auto ptr: widgets) {
    int w1 = ptr->width();
    if (x+w1>w && x>0) {
      y += lh + 1;
      x = 0;
      lh = 0;
    }
    int h = ptr->height();
    if (h>lh)
      lh = h;
    x += w1 + 8;
  }

  y += lh + 1;
  x = 0;
  lh = editor->height();
  
  return y + lh;
}

void AppliedTagList::editorAction() {
  QString tag = editor->text();
  QSet<int> ids = tags.smartFindAll(tag);
  int id = ids.isEmpty() ? tags.define(tag) 
    : ids.size()==1 ? *ids.begin()
    : 0;
  if (id==0)
    return; // sorry, cannot do

  QSet<quint64> sel = selection.current();
  for (auto vsn: sel)
    tags.apply(vsn, id);
  rebuild();
}

void AppliedTagList::clickBrowse() {
  dialog->reset();
  dialog->show();
}


