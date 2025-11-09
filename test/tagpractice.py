#!/usr/bin/python3

from PyQt6.QtCore import QSize, QPoint, Qt
from PyQt6.QtGui import  QColor, QPen, QFont, QFontMetrics, QPainter, QTextCursor
from PyQt6.QtWidgets import QWidget, QTextEdit, QApplication, QLabel, QVBoxLayout

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
    

class LineEdit(QTextEdit):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.lister = ListWidget(parent)
        self.lister.hide()
        self.owntext = ""
        #self.textEdited.connect(lambda x: self.change())

    def hideHint(self):
        c = self.textCursor()
        p, a, s = c.position(), c.anchor(), c.hasSelection()
        self.setPlainText(self.owntext)
        if s:
            c.setPosition(a)
            c.setPosition(p, QTextCursor.MoveMode.KeepAnchor)
        else:
            c.setPosition(p)
        self.setTextCursor(c)
        self.lister.clear()
        self.lister.hide()

    def showHint(self):
        t = self.owntext
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
        self.useHint()

    def useHint(self):
        c = self.textCursor()
        k = c.position()
        print("usehint", k)
        t1 = self.lister.text()
        if t1:
            self.setHtml(t1[:len(self.owntext)]
                         + """<span style="color: #808080;">"""
                         + t1[len(self.owntext):]
                         + "</span>")
            c.setPosition(k)
            self.setTextCursor(c)

    def keyPressEvent(self, evt):
        print("keypress", evt.key())
        k = evt.key()
        t = evt.text()
        if k == Qt.Key.Key_Enter or k == Qt.Key.Key_Return:
            self.owntext = self.toPlainText()
            self.hideHint()
            self.enterPress()
        elif k == Qt.Key.Key_Escape:
            if self.lister.text():
                self.hideHint()
            else:
                self.escapePress()
        elif k == Qt.Key.Key_Left:
                super().keyPressEvent(evt)
        elif k == Qt.Key.Key_Right:
            c = self.textCursor()
            if c.position() == len(self.owntext):
                self.owntext = self.toPlainText()
                c.setPosition(len(self.owntext))
                self.setTextCursor(c)
                self.hideHint()
                self.showHint()
            else:
                super().keyPressEvent(evt)
        elif k == Qt.Key.Key_Up:
            self.lister.up()
            self.useHint()
        elif k == Qt.Key.Key_Down:
            self.lister.down()
            self.useHint()
        elif t:
            self.hideHint()
            super().keyPressEvent(evt)
            c = self.textCursor() # In python, this does not copy
            p, a, s = c.position(), c.anchor(), c.hasSelection()
            self.owntext = self.toPlainText()
            self.showHint()
            if s:
                c.setPosition(a)
                c.setPosition(p, QTextCursor.MoveMode.KeepAnchor)
                self.setTextCursor(c)
                c = self.textCursor()
        else:
            super().keyPressEvent(evt)
            

    def enterPress(self):
        print("enter pressed", self.toPlainText())

    def escapePress(self):
        print("escape pressed")

app = QApplication([])
mw = QWidget()
mw.resize(800, 600)
le = LineEdit(mw)
le.resize(200, 50)
le.move(500, 500)
mw.show()
app.exec()
