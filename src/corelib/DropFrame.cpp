// DropFrame.cpp

#include "DropFrame.h"
#include <QDebug>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>

DropFrame::DropFrame(QWidget *parent): QLabel(parent) {
  markFrame(false);
  setAcceptDrops(true);
}

void DropFrame::dragEnterEvent(QDragEnterEvent *e) {
  QStringList fmts = e->mimeData()->formats();
  if (fmts.contains("text/uri-list")
      && !fmts.contains("x-special/photohoard-versionid")) {
    markFrame(true);
    e->acceptProposedAction();
  }
}

void DropFrame::dragLeaveEvent(QDragLeaveEvent *) {
  markFrame(false);
}

void DropFrame::dragMoveEvent(QDragMoveEvent *e) {
  Qt::KeyboardModifiers m = e->keyboardModifiers();
  Qt::DropActions acts = e->possibleActions();
  if ((m & Qt::ShiftModifier) && (acts & Qt::CopyAction)) {
    e->setDropAction(Qt::CopyAction);
    e->accept();
  } else if ((m & Qt::ControlModifier) && (acts & Qt::LinkAction)) {
    e->setDropAction(Qt::LinkAction);
    e->accept();
  } else {
    e->acceptProposedAction();
  }
}

void DropFrame::dropEvent(QDropEvent *e) {
  QStringList fmts = e->mimeData()->formats();
  markFrame(false);
  if (fmts.contains("text/uri-list")
      && !fmts.contains("x-special/photohoard-versionid")) {
    QList<QUrl> urls = e->mimeData()->urls();
    Qt::DropAction act = e->dropAction();
    e->accept();
    
    for (auto u: urls) 
      qDebug() << "drop " << u << act;
      
    emit dropped(urls, act);
  } else {
    e->ignore();
  }
}

void DropFrame::markFrame(bool in) {
  if (in) {
    setFrameShape(QFrame::Panel);
    setFrameShadow(QFrame::Sunken);
  } else {
    setFrameShape(QFrame::Box);
    setFrameShadow(QFrame::Raised);
  }
}

