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
signals:
  void accepted();
  void canceled();
public:
  QString destination() const;
  bool hasMovieDestination() const;
  QString movieDestination() const;
  QString collection() const;
  CopyIn::SourceDisposition sourceDisposition() const;
protected:
  virtual void keyPressEvent(QKeyEvent *);
private slots:
  void updateCounts(int ntotal, int nmov);
  void changeCollection(QString);
  void browseDestination();
  void browseMovieDestination();
  void changeDisposition();
private:
  class Ui_ImportExternalDialog *ui;
private:
  ImportJob *job;
  QString what;
  QString movieWhat;
  bool bkremoved;
};

#endif
