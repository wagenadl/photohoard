// MultiDragDialog.cpp

#include "MultiDragDialog.h"
#include "ui_MultiDragDialog.h"
#include <QDebug>
#include <QProgressDialog>
#include "Exporter.h"
#include <QMessageBox>
#include <QFileInfo>
#include "ExportDialog.h"
#include "Dialog.h"

bool MultiDragDialog::dontshow = false;

MultiDragDialog::MultiDragDialog(SessionDB *db,
                                 Exporter *expo,
                                 QSet<quint64> const &set):
  db(db), exporter(expo), set(set) {
  ui = new Ui_MultiDragDialog;
  ui->setupUi(this);
  connect(ui->dropParentFolder, SIGNAL(dropped(QString)),
          SLOT(dropParentFolder(QString)));
  connect(ui->dropSibling, SIGNAL(dropped(QString)),
          SLOT(dropSibling(QString)));
  setAttribute(Qt::WA_DeleteOnClose);
  QString msg = ui->label->text();
  if (set.size()==1) {
    msg.replace("#n", "one");
    msg.replace("#were", "was");
  } else {
    msg.replace("#n", QString::number(set.size()));
    msg.replace("#were", "were");
  }
  ui->label->setText(msg);
  ui->dropParentFolder->setMustBeDir(true);
  Dialog::ensureSize(this);
}

MultiDragDialog::~MultiDragDialog() {
}

void MultiDragDialog::dropParentFolder(QString dst) {
  QFileInfo fi(dst);
  if (fi.isDir()) {
    copyInto(fi.absoluteFilePath());
  } else {
    qWarning() << "Destination is not a folder";
    return;
  }
}

void MultiDragDialog::dropSibling(QString dst) {
  QFileInfo fi(dst);
  copyInto(fi.absolutePath());
}

void MultiDragDialog::setDontShowAgain(bool b) {
  dontshow = b;
}

void MultiDragDialog::copyInto(QString dst) {
  hide();
  ExportSettings settings = exporter->settings();
  settings.destination = dst;
  exporter->setup(settings);
  exporter->add(set);
  deleteLater();
}

void MultiDragDialog::openExportSettings() {
  ExportDialog::standalone(exporter);
}
