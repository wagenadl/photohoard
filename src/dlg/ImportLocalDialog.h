// ImportLocalDialog.h

#ifndef IMPORTLOCALDIALOG_H

#define IMPORTLOCALDIALOG_H

#include <QWidget>
#include <QList>
#include <QUrl>
#include "CopyIn.h"

class ImportLocalDialog: public QWidget {
  Q_OBJECT;
public:
  ImportLocalDialog(class ImportJob *job,
                    QStringList collections,
                    QWidget *parent=0);
  virtual ~ImportLocalDialog();  
signals:
  void accepted();
  void canceled();
public:
  QString destination() const;
  QString collection() const;
  bool importInstead() const;
  bool hasMovieDestination() const;
  QString movieDestination() const;
  CopyIn::SourceDisposition sourceDisposition() const;
protected:
  virtual void keyPressEvent(QKeyEvent *);
private slots:
  void updateCounts(int ntotal, int nmov);
  void changeCollection(QString);
  void browseDestination();
  void browseMovieDestination();
  void changeDisposition();
  void changeMode(bool);
private:
  class Ui_ImportLocalDialog *ui;
private:
  ImportJob *job;
  QString what;
  QString movieWhat;
  bool bkremoved;
};

#endif
