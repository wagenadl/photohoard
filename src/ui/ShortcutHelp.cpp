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

void SHPrivate::rebuild() {
  html = "<html><head><style>";
  html += "";
  html += "</style></head>\n";
  
  html += "<body>";

  html += "<h1>Keyboard shortcuts</h1>\n";
  
  for (int n=0; n<sectionNames.size(); n++) {
    html += "<h2>" + sectionNames[n] + "</h2>";
    html += "<table>";
    QList<Action> const &acts(sectionContents[n]->all());
    for (auto a: acts) {
      QString doc = a.documentation();
      if (doc.isEmpty())
        continue;
      html += "<tr>";
      html += "<td>" + a.keyName() + "</td>";
      html += "<td>" + doc + "</td>";
      html += "</tr>";
    }
    html += "</table>\n";
  }
  html += "</body></html>\n";

  editor->document()->setHtml(html);
}

//////////////////////////////////////////////////////////////////////

ShortcutHelp::ShortcutHelp(QWidget *parent): QWidget(parent) {
  d = new SHPrivate(this);
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
