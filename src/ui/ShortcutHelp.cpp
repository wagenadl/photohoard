// ShortcutHelp.cpp

#include "ShortcutHelp.h"
#include "Action.h"
#include <QStringList>
#include <QTextEdit>
#include <QVBoxLayout>
#include "PDebug.h"
#include "Version.h"

class SHEditor: public QTextEdit {
public:
  QSize sizeHint() const override {
    return document()->size().toSize();
  }
  void setHtml(QString html) {
    document()->setHtml(html);
    document()->setDocumentMargin(12);
    updateGeometry();
  }
};

class SHPrivate {
public:
  SHPrivate(ShortcutHelp *parent);
  void rebuild();
public:
  QVBoxLayout *layout;
  QStringList sectionNames;
  QList<Actions const *> sectionContents;
  ShortcutHelp *parent;
  QString html;
  SHEditor *editor;
};

SHPrivate::SHPrivate(ShortcutHelp *parent): parent(parent) {
  layout = new QVBoxLayout;
  layout->setMargin(0);
  editor = new SHEditor;
  editor->setReadOnly(true);
  layout->addWidget(editor);
  parent->setLayout(layout);
  rebuild();
}

static QString htmlify(QString x) {
  x.replace("&", "&amp;");
  x.replace("<", "&lt;");
  x.replace(">", "&gt;");
  x.replace("-", "&minus;");
  return x;
}

void SHPrivate::rebuild() {
  html = "<html><head><style>";
  html += "";
  html += "</style></head>\n";
  
  html += "<body>";

  html += "<h2>Photohoard " + Version::toString() + "</h2>\n";
  html += "<p>(C) Daniel Wagenaar 2016–2023\n";

  html += "<h2>Keyboard shortcuts</h2>\n";

  int N = sectionNames.size();
  
  QStringList sectionCores;
  QList<int> sectionCounts;
  for (int n=0; n<N; n++) {
    int count = 0;
    QString html;

    QList<Action> const &acts(sectionContents[n]->all());
    for (auto a: acts) {
      QString doc = htmlify(a.documentation());
      if (doc.isEmpty())
        continue;
      QString kn = htmlify(a.keyName());
      kn.replace("Ctrl+Shift", "Ctl+Sh.");
      html += "<tr>";
      html += "<td width=\"160\">" + kn + "</td>";
      html += "<td width=\"370\">" + doc + "</td>";
      html += "</tr>";
      count ++;
    }
    sectionCores << html;
    sectionCounts << count;
  }
  QList<int> cumsum;
  int cs=0;
  for (int n=0; n<N; n++) {
    cs += sectionCounts[n];
    cumsum << cs;
  }

  html += "<table><tr><td width=\"400\">";
  html += "<table>";
  int col = 0;
  for (int n=0; n<N; n++) {
    if (n==0 || sectionNames[n]!=sectionNames[n-1])
      html += "<tr></tr><tr><td colspan=\"2\"><b>" + sectionNames[n] + "</b></td></tr>";
    html += sectionCores[n];
    if (cumsum[n]>cs*(col+1)/3) {
      col++;
      html += "</table>";
      html += "</td><td width=\"400\">";
      html += "<table>";
    }
  }
  html += "</table>";
  html += "</td></tr></table>";

  html += "<h2>More information</h2>";
  html += "More information, including a user manual, is available at www.danielwagenaar.net/photohoard.";

  html += "<p><b>License terms</b>";

  html += "<p><small>Photohoard is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.<br>\n";
  html += "Photohoard is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.<br>\n";
  html += "You should have received a copy of the GNU General Public License along with this program. If not, see www.gnu.org/licenses/gpl-3.0.en.html.\n";
  html += "</body></html>\n";

  editor->setHtml(html);
}

//////////////////////////////////////////////////////////////////////

ShortcutHelp::ShortcutHelp(QWidget *parent): QWidget(parent) {
  d = new SHPrivate(this);
  setWindowTitle("Photohoard Help");
}

ShortcutHelp::~ShortcutHelp() {
  delete d;
}

void ShortcutHelp::addSection(QString label, Actions const &contents) {
  d->sectionNames << label;
  d->sectionContents << &contents;
  d->rebuild();
}

QSize ShortcutHelp::sizeHint() const {
  return QWidget::sizeHint();
}

void ShortcutHelp::resizeEvent(QResizeEvent *) {
  d->rebuild();
}
