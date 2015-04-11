// AppliedTagEditor.cpp

#include "AppliedTagEditor.h"
#include <QMouseEvent>
#include <QKeyEvent>
#include <QApplication>
#include <QClipboard>
#include <QFontMetrics>
#include "PDebug.h"
#include <QPainter>

AppliedTagEditor::AppliedTagEditor(PhotoDB *db, QWidget *parent):
  QFrame(parent), tags(db) {
  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  setContentsMargins(2, 2, 2, 2);
  resize(sizeHint());
  cursorpos = 0;
  selend = -1;
  setAutoFillBackground(true);
}

QString AppliedTagEditor::text() const {
  return txt;
}

void AppliedTagEditor::reset() {
  setText("");
}


void AppliedTagEditor::setText(QString s) {
  txt = s;
  if (cursorpos>s.size())
    cursorpos = s.size();
  selend = -1;
  updateGeometry();
  update();
}

void AppliedTagEditor::mousePressEvent(QMouseEvent *e) {
  if (e->button()==Qt::LeftButton)
    setFocus();
}

void AppliedTagEditor::keyPressEvent(QKeyEvent *e) {
  Qt::KeyboardModifiers m = e->modifiers();
  QString t = e->text();
  bool take = true;
  switch (e->key()) {
  case Qt::Key_Escape:
    clearFocus();
    reset();
    pDebug() << "ATE: Escape";
    break;
  case Qt::Key_Return: case Qt::Key_Enter:
    pDebug() << "ATE: Return" << text();
    emit returnPressed();
    break;
  case Qt::Key_Left:
    if (m & Qt::ShiftModifier) {
      if (selend<0)
	selend = cursorpos;
      if (selend>0)
	selend--;
    } else {
      if (cursorpos>0)
	cursorpos--;
      selend=-1;
    }
    update();
    break;
  case Qt::Key_Right:
    if (m & Qt::ShiftModifier) {
      if (selend<0)
	selend = cursorpos;
      if (selend<txt.size())
	selend++;
    } else {
      if (cursorpos<txt.size())
	cursorpos++;
      selend=-1;
    }
    update();
    break;
  case Qt::Key_A:
    if (m & Qt::ControlModifier) {
      selend=0;
      cursorpos = txt.size();
      update();
    } else {
      take = false;
    }
    break;
  case Qt::Key_Backspace:
    if (selend>=0)
      deleteSelection();
    else if (cursorpos>0) {
      txt = txt.left(cursorpos-1) + txt.mid(cursorpos);
      cursorpos--;
      updateGeometry();
      update();
    }
    break;
  case Qt::Key_Delete:
    if (selend>=0)
      deleteSelection();
    else if (cursorpos<txt.size()) {
      txt = txt.left(cursorpos);
      updateGeometry();
      update();
    }
    break;
  case Qt::Key_C:
    if (m & Qt::ControlModifier) {
      if (selend>=0)
	QApplication::clipboard()->setText(txt.mid(selectionStart(),
						   selectionEnd()));
    } else {
      take = false;
    }
    break;
  case Qt::Key_X:
    if (m & Qt::ControlModifier) {
      if (selend>=0) {
	QApplication::clipboard()->setText(txt.mid(selectionStart(),
						   selectionEnd()));
	deleteSelection();
      }
    } else {
      take = false;
    }
    break;
  case Qt::Key_V:
    if (m & Qt::ControlModifier) {
      deleteSelection();
      txt = txt.left(cursorpos) + QApplication::clipboard()->text()
	+ txt.mid(cursorpos);
      updateGeometry();
      update();
    } else {
      take = false;
    }
    break;
  case Qt::Key_Home:
    if (m & Qt::ShiftModifier) 
      selend = 0;
    else
      cursorpos = 0;
    update();
    break;
  case Qt::Key_End:
    if (m & Qt::ShiftModifier) 
      selend = txt.size();
    else
      cursorpos = txt.size();
    update();
    break;
  default:
    take = false;
    break;
  }

  pDebug() << "ATE: " << take << t;
  if (!take && !t.isEmpty()) {
    deleteSelection();
    txt = txt.left(cursorpos) + t + txt.mid(cursorpos);
    cursorpos += t.size();
    updateGeometry();
    update();
    take = true;
  }
  
  if (take) {
    e->accept();
    resize(sizeHint()); // hmmm.
  } else {
    e->ignore();
  }
}

int AppliedTagEditor::selectionStart() {
  return cursorpos<selend ? cursorpos:selend;
}

int AppliedTagEditor::selectionEnd() {
  return selend>cursorpos ? selend:cursorpos;
}
 
void AppliedTagEditor::deleteSelection() {
  if (selend<0)
    return;
  txt = txt.left(selectionStart()) + txt.mid(selectionEnd());
  if (cursorpos>selend)
    cursorpos=selend;
  selend = -1;
  updateGeometry();
  update();
}

QSize AppliedTagEditor::sizeHint() const {
  QFontMetrics fm(font());
  QRect r0 = fm.boundingRect("New tag... ");
  QRect r1 = fm.boundingRect(txt + "  ");
  QSize s0 = (r0|r1).size();
  QMargins mm = contentsMargins();
  return s0 + QSize(mm.left()+mm.right(), mm.top()+mm.bottom());
}

QSize AppliedTagEditor::minimumSizeHint() const {
  QFontMetrics fm(font());
  QSize s0 = fm.boundingRect("Hello|").size();
  QMargins mm = contentsMargins();
  return s0 + QSize(mm.left()+mm.right(), mm.top()+mm.bottom());
}


  
void AppliedTagEditor::paintEvent(QPaintEvent *) {
  QRect r = contentsRect();
  QPainter p(this);
  if (selend>=0) {
    QFontMetrics fm(p.font());
    int l = fm.width(txt.left(selectionStart()));
    int l1 = fm.width(txt.left(selectionEnd()));
    QRect sel = QRect(r.topLeft() + QPoint(l, 0),
		      r.bottomLeft() + QPoint(l1, 0));
    p.setBrush(QColor("#888800"));
    p.drawRect(sel);
  }
  p.setBrush(QBrush(Qt::NoBrush));
  drawText(p);
  if (hasFocus()) {
    QFontMetrics fm(p.font());
    int l = fm.width(txt.left(cursorpos));
    QRect rl(r.topLeft() + QPoint(l-1, 0),
	     r.bottomLeft() + QPoint(l+1, 0));
    p.setPen(QColor("#440000")) ;
    p.drawText(rl, Qt::AlignHCenter | Qt::AlignVCenter, "|");
  } else {
    if (txt.isEmpty()) {
      p.setPen(QColor("#444444"));
      QFont f = p.font();
      f.setItalic(true);
      p.setFont(f);
      p.drawText(r, Qt::AlignLeft | Qt::AlignVCenter,
		 QString::fromUtf8("New tag…"));
    }
  }
}

void AppliedTagEditor::drawText(QPainter &p) {
  if (txt.isEmpty())
    return;
  QSet<int> ids = tags.smartFindAll(txt);
  if (ids.isEmpty()) {
    if (tags.couldBeNew(txt))
      p.setPen(QColor("#660000"));
    else
      p.setPen(QColor("#ff0000"));
  } else if (ids.size()==1) {
    p.setPen(QColor("#008800"));
  } else {
    p.setPen(QColor("#000000"));
  }
  p.drawText(contentsRect(), Qt::AlignLeft | Qt::AlignVCenter, txt);
}  

void AppliedTagEditor::focusInEvent(QFocusEvent *) {
  update();
}

void AppliedTagEditor::focusOutEvent(QFocusEvent *) {
  update();
}


