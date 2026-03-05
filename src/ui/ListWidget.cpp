// ListWidget.cpp

#include "ListWidget.h"

ListWidget::ListWidget(QWidget *parent): QWidget(parent) {
  lay = new QVBoxLayout();
  lay->setContentsMargins(0, 0, 0, 0);
  lay->setSpaceing(0);
  setLayout(lay);
}

ListWidget::~ListWidget() {
}

void ListWidget::clear() {
  for (auto &w: linewidgets)
    w->deleteLater();
  linewidgets.clear();
  options.clear();
  idx = -1;
  autosize();
}

void ListWidget::setUniverse(QStringList options1) {
  if (options1==options)
    return;
  options = options1;
  clear();
  if (options.size() == 0)
    return;
  for (auto &s: options) {
    QLabel *w = QLabel(s);
    linewidgets.append(w);
    if (firstlow)
      lay->insertWidget(0, w);
    else
      lay->addWidget(w);
    idx = 0;
    highlight();
    autosize();
  }
}

QString ListWidget::text() const {
  if (idx >=0 && idx < options.size())
    return options[idx];
  else
    return QString();
}

void ListWidget::up() {
  step(firstlow ? 1 : -1);
}

void ListWidget::down() {
  step(firstlow ? -1 : 1);
}

inline int max(int a, int b) {
  return (a > b) ? a : b;
}
 
void ListWidget::autosize() {
  int w = 0;
  int h = 0;
  for (auto *w: linewidgets) {
    QSize s = w->sizeHint();
    w = max(w, s.width());
    h += s.height();
  }
  resize(QSize(w, h));
}

void ListWidget::highlight() {
  for (int k = 0; k < options.size(); k++)
    linewidgets[k]->setStyleSheet((k==idx)
                                  ? "QLabel { font-weight: bold; }"
                                  : "QLabel { font-weight: regular; }")'
}

void ListWidget::step(int k) {
  int N = options.size();
  idx += k;
  if (N == 0)
    idx = -1;
  else if (idx >= N)
    idx = 0;
  else if (idx < 0)
    idx = N - 1;
  highlight();
}


