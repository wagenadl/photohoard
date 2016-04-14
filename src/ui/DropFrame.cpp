// DropFrame.cpp

#include "DropFrame.h"
#include <QDebug>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QFileInfo>
#include "PDebug.h"

DropFrame::DropFrame(QWidget *parent): QLabel(parent) {
  markFrame(false);
  setAcceptDrops(true);
  setMustBeDir(false);
}

void DropFrame::setMustBeDir(bool b) {
  mustbedir = b;
}

void DropFrame::dragLeaveEvent(QDragLeaveEvent *) {
  markFrame(false);
}

void DropFrame::dragEnterEvent(QDragEnterEvent *e) {
  QMimeData const *data = e->mimeData();
  QStringList fmts = data->formats();
  if (fmts.contains("text/uri-list")
      && !fmts.contains("x-special/photohoard-versionid")) {
    QList<QUrl> urls = data->urls();
    if (urls.size()==1 && urls.first().isLocalFile()) {
      if (!mustbedir || QFileInfo(urls.first().path()).isDir()) {
        markFrame(true);
        e->acceptProposedAction();
      }
    }
  }
}

void DropFrame::dropEvent(QDropEvent *e) {
  QStringList fmts = e->mimeData()->formats();
  markFrame(false);
  QList<QUrl> urls = e->mimeData()->urls();
  ASSERT(urls.size()==1);
  e->accept();
  emit dropped(urls.first().path());
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

