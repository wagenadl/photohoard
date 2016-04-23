// ImportExternalDialog.h

#ifndef IMPORTEXTERNALDIALOG_H

#define IMPORTEXTERNALDIALOG_H

#include <QWidget>
#include <QList>
#include <QUrl>
#include "CopyIn.h"

class ImportExternalDialog: public QWidget {
  Q_OBJECT;
public:
  ImportExternalDialog(class ImportJob *job,
                       QStringList collections,
                       QWidget *parent=0);
  virtual ~ImportExternalDialog();
  void showAndGo();
signals:
  void accepted();
  void canceled();
public:
  QString destination() const;
  bool hasMovieDestination() const;
  QString movieDestination() const;
  QString collection() const;
  CopyIn::SourceDisposition sourceDisposition() const;
private slots:
  void updateCounts(int ntotal, int nmov);
  void changeCollection(QString);
  void browseDestination();
  void browseMovieDestination();
private:
  class Ui_ImportExternalDialog *ui;
private:
  ImportJob *job;
  QString what;
  QString movieWhat;
};

#endif
