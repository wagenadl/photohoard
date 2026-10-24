// ShortcutHelp.cpp

#include "ShortcutHelp.h"
#include "Action.h"
#include <QStringList>
#include <QTextEdit>
#include <QVBoxLayout>
#include <qnamespace.h>
#include "Style.h"
#include "Version.h"

class SHEditor: public QTextEdit {
public:
  SHEditor(QWidget *parent=0): QTextEdit(parent) {
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  }
  QSize sizeHint() const override {
    return document()->size().toSize();
  }
  void setHtml(QString html) {
    document()->setDefaultStyleSheet(Style::instance()->documentStyle());
    document()->setHtml(html);
    document()->setDocumentMargin(12);
    updateGeometry();
  }
};

class SHPrivate {
public:
  SHPrivate();
  QString html(QFont const &);
public:
  QStringList sectionNames;
  QList<Actions const *> sectionContents;
};

SHPrivate::SHPrivate() {
}

static QString htmlify(QString x) {
  x.replace("&", "&amp;");
  x.replace("<", "&lt;");
  x.replace(">", "&gt;");
  x.replace("-", "&minus;");
  return x;
}

QString SHPrivate::html(QFont const &f) {
  qDebug() << "font" << f.family() << f.pointSize() << f.pixelSize();
  QFontMetrics fm(f);
  int keywid = fm.boundingRect("Ctl.+Sh.+@xxx").width();
  int valwid = fm.boundingRect("Decrease value (try Alt, Shift)xxxxxxxx").width();
  int mulwid = keywid + valwid;
  QString year = QString("%1").arg(Version::buildDate().year());
  QString html_;
  html_ = "<html><head><style>";
  html_ += "td { margin-right: 10px; }";
  html_ += "</style></head>\n";
  
  html_ += "<body>";

  html_ += "<h2>Photohoard " + Version::toString() + "</h2>\n";
  html_ += "<p>(C) Daniel Wagenaar 2016–" + year + "\n";

  html_ += "<h2>Keyboard shortcuts</h2>\n";

  int N = sectionNames.size();
  
  QStringList sectionCores;
  QList<int> sectionCounts;
  for (int n=0; n<N; n++) {
    int count = 0;
    QString html_;

    QList<Action> const &acts(sectionContents[n]->all());
    for (auto a: acts) {
      QString doc = htmlify(a.documentation());
      if (doc.isEmpty())
        continue;
      QString kn = htmlify(a.keyName());
      kn.replace("Ctrl+Shift", "Ctl+Sh.");
      html_ += "<tr>";
      html_ += QString("<td width=\"%1\">" + kn + "</td>").arg(keywid);
      html_ += QString("<td width=\"%1\">" + doc + "</td>").arg(valwid);
      html_ += "</tr>";
      count ++;
    }
    sectionCores << html_;
    sectionCounts << count;
  }
  QList<int> cumsum;
  int cs=0;
  for (int n=0; n<N; n++) {
    cs += sectionCounts[n];
    cumsum << cs;
  }

  html_ += QString("<table><tr><td width=\"%1\">").arg(mulwid);
  html_ += "<table>";
  int col = 0;
  for (int n=0; n<N; n++) {
    if (n==0 || sectionNames[n]!=sectionNames[n-1])
      html_ += "<tr></tr><tr><td colspan=\"2\"><b>" + sectionNames[n] + "</b></td></tr>";
    html_ += sectionCores[n];
    if (cumsum[n]>cs*(col+1)/3) {
      col++;
      html_ += "</table>";
      html_ += QString("</td><td width=\"%1\">").arg(mulwid);
      html_ += "<table>";
    }
  }
  html_ += "</table>";
  html_ += "</td></tr></table>";

  html_ += "<h2>More information</h2>";
  html_ += "Full documentation is available at <a href=\"https://photohoard.readthedocs.io\">photohoard.readthedocs.io</a>.";

  html_ += "<p><b>License terms</b>";

  html_ += "<p><small>Photohoard is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.<br>\n";
  html_ += "Photohoard is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.<br>\n";
  html_ += "You should have received a copy of the GNU General Public License along with this program. If not, see www.gnu.org/licenses/gpl-3.0.en.html.\n";
  html_ += "</body></html>\n";
  return html_;
}

//////////////////////////////////////////////////////////////////////

ShortcutHelp::ShortcutHelp() { d = new SHPrivate(); }

ShortcutHelp::~ShortcutHelp() { delete d; }

QString ShortcutHelp::html(QFont const &f) const {
  return d->html(f);
}

ShortcutHelpDialog::ShortcutHelpDialog(ShortcutHelp *data, QWidget *parent):
  QDialog(parent) {
  layout = new QVBoxLayout;
  layout->setContentsMargins(0, 0, 0, 0);
  editor = new SHEditor;
  editor->setHtml(data->html(font()));
  editor->setReadOnly(true);
  layout->addWidget(editor);
  setLayout(layout);
}

ShortcutHelpDialog::~ShortcutHelpDialog() {}

QSize ShortcutHelpDialog::sizeHint() const {
  return editor->sizeHint() + QSize(4, 4);
}
    
void ShortcutHelp::addSection(QString label, Actions const &contents) {
  d->sectionNames << label;
  d->sectionContents << &contents;
}

void ShortcutHelpDialog::focusOutEvent(QFocusEvent *) {
  close();
}
