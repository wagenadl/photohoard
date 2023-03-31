// CollapsingGroupBox.cpp

#include "CollapsingGroupBox.h"
#include <QVBoxLayout>
#include <QDebug>

CollapsingGroupBox::CollapsingGroupBox(QWidget *parent):
  QGroupBox(parent) {
  _open = true;
}


CollapsingGroupBox::CollapsingGroupBox(QString const &title,
                                       QWidget *parent):
  CollapsingGroupBox(parent) {
  setTitle(title);
}

CollapsingGroupBox::~CollapsingGroupBox() {
}

void CollapsingGroupBox::setTitle(QString ttl) {
  _title = ttl;
  QGroupBox::setTitle(_open ? openTitle() : closedTitle());
}

QString CollapsingGroupBox::title() const {
  return _title;
}

QString CollapsingGroupBox::openTitle() const {
  return "▲ " + _title;
}

QString CollapsingGroupBox::closedTitle() const {
  return "▼ " + _title;
}

void CollapsingGroupBox::collapse() {
  if (!_open)
    return;
  QLayout *lay = layout();
  if (lay) {
    mrg = lay->contentsMargins();
    sp = lay->spacing();
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);
  }

  for (QWidget *w: findChildren<QWidget *>(QString(),
                                           Qt::FindDirectChildrenOnly)) {
    wvis[w] = w->isVisibleTo(this);
    w->hide();
  }
  QGroupBox::setTitle(closedTitle());
  _open = false;
}

void CollapsingGroupBox::open() {
  if (_open)
    return;
  QLayout *lay = layout();
  if (lay) {
    lay->setContentsMargins(mrg);
    lay->setSpacing(sp);
  }
  for (QWidget *w: findChildren<QWidget *>(QString(),
                                           Qt::FindDirectChildrenOnly)) {
    if (wvis[w])
      w->show();
  }
  QGroupBox::setTitle(openTitle());
  _open = true;
}

void CollapsingGroupBox::mouseDoubleClickEvent(QMouseEvent *) {
  qDebug() << "double click";
  if (_open)
    collapse();
  else
    open();
}

