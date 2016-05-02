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
private slots:
  void changeCollection(QString);
  void browseDestination();
private:
  class Ui_ImportLocalDialog *ui;
private:
  ImportJob *job;
};

#endif
