// MultiDragDialog.cpp

#include "MultiDragDialog.h"
#include "ui_MultiDragDialog.h"
#include <QDebug>
#include <QProgressDialog>
#include "Exporter.h"
#include <QMessageBox>
#include <QFileInfo>

bool MultiDragDialog::dontshow = false;

MultiDragDialog::MultiDragDialog(SessionDB *db,
                                 QSet<quint64> const &set):
  db(db), set(set) {
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
  progress = 0;
  exporter = 0;
}

MultiDragDialog::~MultiDragDialog() {
}

void MultiDragDialog::dropParentFolder(QString dst) {
  QFileInfo fi(dst);
  if (fi.isDir()) {
    copyInto(fi.absoluteFilePath());
  } else {
    qDebug() << "Destination is not a folder";
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

  progress = new QProgressDialog(QString("Copying %1 %2 to %3...")
                                 .arg(set.size())
                                 .arg(set.size()==1 ? "image" : "images")
                                 .arg(dst), "Cancel",
                                 0, set.size());
  progress->setWindowTitle("Photohoard");
  progress->setValue(0);

  exporter = new Exporter(db, this);
  connect(exporter, SIGNAL(progress(int,int)),
          SLOT(exportProgress(int,int)));
  connect(exporter, SIGNAL(completed(QString,int,int)),
          SLOT(exportComplete(QString,int,int)));
  ExportSettings settings;
  settings.destination = dst;
  exporter->setup(settings);
  exporter->start();
  exporter->add(set);
}

void MultiDragDialog::exportProgress(int n, int) {
  progress->setValue(n);
  if (progress->wasCanceled()) {
    exporter->stop();
    progress->deleteLater();
    deleteLater();
  }
}

void MultiDragDialog::exportComplete(QString, int nok, int) {
  delete progress;
  progress = 0;
  exporter->stop();
  if (nok < set.size()) 
    QMessageBox::warning(0, "Photohoard",
                         QString("Export failed: %1 image(s) not exported")
                         .arg(set.size()-nok));
  deleteLater();
}
