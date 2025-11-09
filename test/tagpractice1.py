#!/usr/bin/python3

from PyQt6.QtCore import QSize, QPoint, Qt
from PyQt6.QtGui import  QColor, QPen, QFont, QFontMetrics, QPainter
from PyQt6.QtWidgets import QWidget, QLineEdit, QApplication, QLabel, QVBoxLayout

tags = ["Hello", "World", "Sam", "Alexander", "Somebody", "Something", "Some"]
tags.sort()

class ListWidget(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.lay = QVBoxLayout()
        self.lay.setContentsMargins(0, 0, 0, 0)
        self.lay.setSpacing(0)
        self.setLayout(self.lay)
        self.widgets = []
        self.tags = []
        self.idx = -1
        self.setStyleSheet("QWidget { border: 1px solid green }")
        
    def clear(self):
        for w in self.widgets:
            w.deleteLater()
        self.widgets = []
        self.tags = []
        self.idx = -1
        self.autosize()
    
    def set(self, tags):
        tags = tags[-1::-1]
        if tags == self.tags:
            return
        self.clear()
        for t in tags:
            self.tags.append(t)
            w = QLabel(t)
            self.widgets.append(w)
            self.lay.addWidget(w)
        self.idx = len(tags) - 1
        self.highlight()
        self.autosize()

    def autosize(self):
        w = 0
        h = 0
        for wdg in self.widgets:
            s = wdg.sizeHint()
            print("widget", s)
            w = max(w, s.width())
            h += s.height()
        self.resize(QSize(w, h))

    def highlight(self):
        for idx, w in enumerate(self.widgets):
            if idx == self.idx:
                w.setStyleSheet("QLabel { font-weight: bold; }")
            else:
                w.setStyleSheet("QLabel { font-weight: regular; }")

    def up(self):
        self.idx -= 1
        if self.idx < 0:
            self.idx = len(self.tags) - 1
        self.highlight()

    def down(self):
        self.idx += 1
        if self.idx >= len(self.tags):
            self.idx = 0
        self.highlight()

    def text(self):
        if 0 <= self.idx < len(self.tags):
            return self.tags[self.idx]
        return ""
    

class LineEdit(QLabel):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.lister = ListWidget(parent)
        self.lister.hide()
        #self.textEdited.connect(lambda x: self.change())
        self.showcursor = False
        self.text = ""
        self.idx = 0
        self.setAutoFillBackground(True)
        self.startTimer(500)

    def sizeHint(self):
        return QSize(200, 50)

    def timerEvent(self, evt):
        self.showcursor = not self.showcursor
        self.update()

    def keyPressEvent(self, evt):
        print("keypress", evt.key())
        k = evt.key()
        t = evt.text()
        if k == Qt.Key.Key_Enter or k == Qt.Key.Key_Return:
            t = self.lister.text()
            if t:
                self.text = t
                self.idx = len(self.text)
            self.lister.clear()
            self.lister.hide()
            self.enterPress()
        elif k == Qt.Key.Key_Escape:
            if self.lister.text():
                self.lister.clear()
                self.lister.hide()
                self.update()
            else:
                self.escapePress()
        elif k == Qt.Key.Key_Up:
            self.lister.up()
            self.update()
        elif k == Qt.Key.Key_Down:
            self.lister.down()
            self.update()
        elif k == Qt.Key.Key_Backspace:
            if self.idx > 0:
                self.idx -= 1
                self.text = self.text[:self.idx] + self.text[self.idx+1:]
                self.change()
        elif k == Qt.Key.Key_Delete:
            if self.idx < len(self.text):
                self.text = self.text[:self.idx] + self.text[self.idx+1:]
                self.change()                    
        elif t:
            self.text = self.text[:self.idx] + t + self.text[self.idx:]
            self.idx += len(t)
            self.change()

    def mousePressEvent(self, evt):
        self.setFocus()

    def paintEvent(self, evt):
        p = QPainter()
        p.begin(self)
        f = self.font()
        if self.text == "" and not self.hasFocus():
            p.setPen(QPen(QColor(100, 100, 100)))
            f.setStyle(QFont.Style.StyleItalic)
            p.setFont(f)
            p.drawText(5, 40, "Add tag…")
        else:
            p.setPen(QPen(QColor(0, 0, 0)))
            f.setStyle(QFont.Style.StyleNormal)
            p.setFont(f)
            p.drawText(5, 40, self.text)
            if self.showcursor:
                fm = QFontMetrics(f)
                dx = fm.horizontalAdvance(self.text[:self.idx])
                dx1 = fm.horizontalAdvance('|')
                p.drawText(5 + dx - dx1//2, 40, '|')
            if self.lister.idx >= 0:
                p.setPen(QPen(QColor(100, 100, 100)))
                fm = QFontMetrics(f)
                p.drawText(5 + fm.horizontalAdvance(self.text), 40,
                           self.lister.text()[self.idx:])
        p.end()

    def enterPress(self):
        print("enter pressed", self.text)

    def escapePress(self):
        print("escape pressed")

    def change(self):
        t = self.text
        print("change", t)
        if len(t):
            cands = [t1 for t1 in tags if t1.lower().startswith(t.lower())]
            self.lister.set(cands)
            if len(cands):
                self.lister.show()
                self.lister.move(self.pos()
                                 - QPoint(0, self.lister.height()))
            else:
                self.lister.hide()
        else:
            self.lister.hide()
        self.update()

app = QApplication([])
mw = QWidget()
mw.resize(800, 600)
le = LineEdit(mw)
le.resize(200, 50)
le.move(500, 500)
mw.show()
app.exec()
