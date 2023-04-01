// RecentFiles.cpp

#include "RecentFiles.h"
#include <QDebug>
#include "Settings.h"
#include <QFileInfo>
#include "FileLocations.h"

RecentFiles::RecentFiles(QString varname, QWidget *parent, QString mydbfn):
  QMenu("&Recent databases", parent),
  varname(varname), mydbfn(mydbfn) {
  for (int n=0; n<MAXFILES; n++) {
    actions[n] = new QAction(this);
    actions[n]->setVisible(false);
    connect(actions[n], &QAction::triggered,
	    [this, n]() {
	      QString fn = actions[n]->data().toString();
	      if (!fn.isEmpty())
		emit selected(fn);
	      qDebug() << "recentfiles" << fn;
	    });
    addAction(actions[n]);
  }
  updateItems();
}

void RecentFiles::mark(QString fn) {
  QStringList files = list();
  fn = QFileInfo(fn).absoluteFilePath();
  files.removeAll(fn);
  files.prepend(fn);
  while (files.size() > MAXFILES)
    files.removeLast();
  Settings().set(varname, files);
  updateItems();
}

QStringList RecentFiles::list() const {
  QStringList lst = Settings().get(varname).toStringList();
  QString dflt = FileLocations::defaultDBFile();
  lst.removeAll(dflt);
  if (QFileInfo(dflt).exists())
    lst.insert(0, dflt);
  return lst;
}

void RecentFiles::updateItems() {
  QStringList files = list();
  files.removeAll(mydbfn);
  QSet<QString> leaves;
  QSet<QString> dups;
  for (int n=0; n<MAXFILES; n++) {
    if (n<files.size()) {
      QString leaf = QFileInfo(files[n]).fileName();
      if (leaves.contains(leaf))
        dups << leaf;
      else
        leaves << leaf;
    }
  }
  for (int n=0; n<MAXFILES; n++) {
    if (n<files.size()) {
      QFileInfo fi(files[n]);
      QString leaf = fi.fileName();
      QString fn = dups.contains(leaf) ? fi.canonicalFilePath() : leaf;
      if (fn.endsWith(".photohoard"))
        fn = fn.left(fn.size() - 11);
      else if (fn.endsWith(".db"))
        fn = fn.left(fn.size() - 3);
      if (fn=="default")
        fn = "Default database";
      else
        fn = "“" + fn + "”";
      QString text = tr("&%1 %2").arg(n + 1).arg(fn);
      actions[n]->setText(text);
      actions[n]->setData(files[n]);
      actions[n]->setVisible(true);
    } else {
      actions[n]->setText("");
      actions[n]->setData(QString());
      actions[n]->setVisible(false);
    }
  }
  qDebug() << "updated";
}

void RecentFiles::showEvent(QShowEvent *) {
  updateItems();
}
