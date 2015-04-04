// TagDialog.cpp

#include "TagDialog.h"
#include <QHBoxLayout>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QSignalMapper>
#include "TagListWidget.h"
#include <QLineEdit>
#include "PDebug.h"
#include <QPushButton>

TagDialog::TagDialog(PhotoDB const &db, bool ro): db(db), tags(db) {
  sw = ShowWhat::AllDefined;
  QVBoxLayout *vlay = new QVBoxLayout();
  QHBoxLayout *hlay = new QHBoxLayout();
  lay = new QGridLayout();
  hlay->addLayout(lay);
  hlay->addStretch();
  vlay->addLayout(hlay);
  QHBoxLayout *blay = new QHBoxLayout();
  blay->addStretch();

  if (ro) {
    addButton = 0;
    deleteButton = 0;
    removeButton = 0;
    applyButton = new QPushButton("Select");
    blay->addWidget(applyButton);
    connect(applyButton, SIGNAL(clicked()), SLOT(clickApply()));
  } else {
    addButton = new QPushButton("Add");
    blay->addWidget(addButton);
    connect(addButton, SIGNAL(clicked()), SLOT(clickAdd()));
    
    deleteButton = new QPushButton("Delete");
    blay->addWidget(deleteButton);
    connect(deleteButton, SIGNAL(clicked()), SLOT(clickDelete()));
    
    blay->addSpacing(6);
    
    removeButton = new QPushButton("Remove");
    blay->addWidget(removeButton);
    connect(removeButton, SIGNAL(clicked()), SLOT(clickRemove()));

    applyButton = new QPushButton("Apply");
    blay->addWidget(applyButton);
    connect(applyButton, SIGNAL(clicked()), SLOT(clickApply()));
  }
  
  blay->addSpacing(6);

  QPushButton *closeButton = new QPushButton("Close");
  blay->addWidget(closeButton);
  connect(closeButton, SIGNAL(clicked()), SLOT(clickClose()));

  vlay->addLayout(blay);

  setLayout(vlay);

  selectionMapper = new QSignalMapper(this);
  connect(selectionMapper, SIGNAL(mapped(int)), SLOT(updateSelection(int)));
  editMapper = new QSignalMapper(this);
  connect(editMapper, SIGNAL(mapped(int)), SLOT(confirmEdit(int)));

  setWindowTitle("Tag manager");

  rebuild(0);
}

TagDialog::~TagDialog() {
}

void TagDialog::reset() {
  rebuild(0);
}

void TagDialog::setShown(TagDialog::ShowWhat s) {
  sw = s;
  for (auto l: lists)
    l->setShown(s);
  rebuild(0); // is this necessary?
}

QList<int> TagDialog::selectedTags() {
  QList<int> res;
  return res;
}

void TagDialog::updateSelection(int lvl) {
  pDebug() << "update selection " << lvl;
  rebuild(lvl+1);
}

int TagDialog::selectedTag(int lvl) const {
  if (lvl<0 || lvl>=lists.size())
    return 0;
  QList<int> sel = lists[lvl]->selectedTags();
  if (sel.isEmpty())
    return 0;
  else
    return sel.first();
}

int TagDialog::terminalTag() const {
  return level==0 ? 0
    : lists[level]->isEmpty() ? selectedTag(level-1)
    : 0;
}

void TagDialog::rebuild(int lvl) {
  level = lvl;
  int root = 0;
  while (level>0) {
    int sel = selectedTag(level-1);
    if (sel==0) {
      level--;
    } else {
      root = sel;
      break;
    }
  }
  
  while (lists.size()<=level) {
    TagListWidget *w = new TagListWidget(db, root);
    w->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    w->setSelectionMode(QAbstractItemView::SingleSelection);
    w->setShown(sw);
    connect(w, SIGNAL(itemSelectionChanged()),
            selectionMapper, SLOT(map()));
    selectionMapper->setMapping(w, lists.size());
    lay->addWidget(w, 0, lists.size());
    lists << w;
  }

  while (editors.size()<=level) {
    QLineEdit *e = new QLineEdit();
    e->setPlaceholderText(QString::fromUtf8("New tag…"));
    connect(e, SIGNAL(returnPressed()),
            editMapper, SLOT(map()));
    connect(e, SIGNAL(textChanged(QString)), SLOT(edited()));
    editMapper->setMapping(e, editors.size());
    lay->addWidget(e, 1, editors.size());
    editors << e;
  }
            
  for (int n=0; n<lists.size(); n++)
    if (n<=level)
      lists[n]->show();
    else
      lists[n]->hide();

  for (int n=0; n<editors.size(); n++) {
    editors[n]->clear();
    if (n<=level)
      editors[n]->show();
    else
      editors[n]->hide();
  }

  lists[level]->load(root);

  updateButtons();
}

void TagDialog::updateButtons() {
  updateAddButton();
  updateDeleteButton();
  updateRemoveButton();
  updateApplyButton();
}

void TagDialog::updateApplyButton() {
  applyButton->setEnabled(false);
  int t = terminalTag();
  if (!t)
    return;
  
  QSqlQuery q = db.query("select version from selection");
  while (q.next()) {
    quint64 vsn = q.value(0).toULongLong();
    if (db.simpleQuery("select count(*) from appliedtags"
                       " where tag==:a and version==:b",
                       t, vsn).toInt()==0) {
      applyButton->setEnabled(true);
      return;
    }
  }
}

void TagDialog::updateRemoveButton() {
  if (!removeButton)
    return;
  
  removeButton->setEnabled(false);
  int t = terminalTag();
  if (!t)
    return;
  
  QSqlQuery q = db.query("select version from selection");
  while (q.next()) {
    quint64 vsn = q.value(0).toULongLong();
    if (db.simpleQuery("select count(*) from appliedtags"
                       " where tag==:a and version==:b",
                       t, vsn).toInt()>0) {
      removeButton->setEnabled(true);
      return;
    }
  }
}

void TagDialog::updateDeleteButton() {
  if (!deleteButton)
    return;
  
  deleteButton->setEnabled(false);
  int t = terminalTag();
  if (t && tags.canUndefine(t))
    deleteButton->setEnabled(true);
}

void TagDialog::edited() {
  updateAddButton();
}

void TagDialog::updateAddButton() {
  if (!addButton)
    return;
  
  int nonempty = 0;
  
  for (int lvl=0; lvl<editors.size(); lvl++) 
    if (!editors[lvl]->text().isEmpty())
      nonempty++;
  
  addButton->setEnabled(nonempty==1);
}

void TagDialog::clickClose() {
  reject();
  close();
}

void TagDialog::clickDelete() {
  int t = terminalTag();
  pDebug() << "clickDelete " << t;
  if (tags.undefine(t)) {
    pDebug() << "deleted";
    rebuild(level-1);
  }
}

void TagDialog::clickAdd() {
  for (int lvl=0; lvl<editors.size(); lvl++) {
    if (!editors[lvl]->text().isEmpty()) {
      confirmEdit(lvl);
      return;
    }
  }
}

void TagDialog::clickApply() {
  int t = terminalTag();
  if (!t)
    return;
  
  emit apply(t);
  if (removeButton) {
    applyButton->setEnabled(false);
    removeButton->setEnabled(true);
  } else {
    accept(); // so we know we are read-only
  }
}

void TagDialog::clickRemove() {
  int t = terminalTag();
  if (!t)
    return;
  
  emit unapply(t);
  applyButton->setEnabled(true);
  removeButton->setEnabled(false);
}

void TagDialog::confirmEdit(int level) {
  QString tag = editors[level]->text();
  if (tag.isEmpty())
    return;
  int root = selectedTag(level-1);
  tags.define(tag, root);
  editors[level]->clear();
  addButton->setEnabled(false);
  rebuild(level);
}

