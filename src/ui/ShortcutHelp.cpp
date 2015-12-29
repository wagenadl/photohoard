// ShortcutHelp.cpp

#include "ShortcutHelp.h"
#include "Action.h"
#include <QStringList>
#include <QTextEdit>
#include <QVBoxLayout>

class SHPrivate {
public:
  SHPrivate(ShortcutHelp *parent);
  void rebuild();
public:
  QStringList sectionNames;
  QList<Actions const *> sectionContents;
  ShortcutHelp *parent;
  QString html;
  QTextEdit *editor;
};

SHPrivate::SHPrivate(ShortcutHelp *parent): parent(parent) {
  auto layout = new QVBoxLayout;
  editor = new QTextEdit;
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
      html += "<tr>";
      html += "<td width=\"120\">" + kn + "</td>";
      html += "<td width=\"250\">" + doc + "</td>";
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
      html += "<tr><td colspan=\"2\"><b>" + sectionNames[n] + "</b></td></tr>";
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

  html += "</body></html>\n";

  editor->document()->setHtml(html);
}

//////////////////////////////////////////////////////////////////////

ShortcutHelp::ShortcutHelp(QWidget *parent): QWidget(parent) {
  d = new SHPrivate(this);
  resize(420*3, 550);
}

ShortcutHelp::~ShortcutHelp() {
  delete d;
}

void ShortcutHelp::addSection(QString label, Actions const &contents) {
  d->sectionNames << label;
  d->sectionContents << &contents;
  d->rebuild();
}

void ShortcutHelp::resizeEvent(QResizeEvent *) {
  d->rebuild();
}
