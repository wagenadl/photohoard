// ImportExternalDialog.h

#ifndef IMPORTEXTERNALDIALOG_H

#define IMPORTEXTERNALDIALOG_H

#include <QDialog>
#include <QList>
#include <QUrl>

class ImportExternalDialog: public QDialog {
  Q_OBJECT;
public:
  ImportExternalDialog(class Scanner *scanner, class SessionDB *db,
                       QList<QUrl> sources,
                       QWidget *parent=0);
  virtual ~ImportExternalDialog();
public:
  static void showAndGo(class Scanner *scanner, class SessionDB *db,
                        QList<QUrl> sources);
private slots:
  void updateCounts(int, int);
  void completeCounts();
  void changeCollection(QString);
  void nowImport();
private:
  class Ui_ImportExternalDialog *ui;
  class Collector *collector;
  class Scanner *scanner;
  SessionDB *db;
  QString what;
  QString movieWhat;
};

#endif
