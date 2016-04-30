// ImportOtherUserDialog.h

#ifndef IMPORTOTHERUSERDIALOG_H

#define IMPORTOTHERUSERDIALOG_H

#include <QWidget>
#include <QList>
#include <QUrl>
#include "CopyIn.h"

class ImportOtherUserDialog: public QWidget {
public:
  ImportOtherUserDialog(class ImportJob *job,
                       QStringList collections,
                       QWidget *parent=0);
  virtual ~ImportOtherUserDialog();
signals:
  void accepted();
  void canceled();
public:
  QString destination() const;
  QString collection() const;
  bool incorporateInstead() const;
private slots:
  void updateCounts(int ntotal, int nmov);
  void changeCollection(QString);
  void browseDestination();
private:
  class Ui_ImportExternalDialog *ui;
private:
  ImportJob *job;
  QString what;
};

#endif
