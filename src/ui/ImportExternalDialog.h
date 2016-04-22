// ImportExternalDialog.h

#ifndef IMPORTEXTERNALDIALOG_H

#define IMPORTEXTERNALDIALOG_H

#include <QWidget>
#include <QList>
#include <QUrl>

class ImportExternalDialog: public QWidget {
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
  void allowImport();
  void startCopy();
  void cancel();
private:
  class Ui_ImportExternalDialog *ui;
  class Collector *collector; // our child
  class Scanner *scanner;
  SessionDB *db;
  QString what;
  QString movieWhat;
  bool accept_prov;
  bool complete_cnt;
  class QProgressDialog *progress; // our responsibility, but not our child
  class CopyIn *copyin; // our child
};

#endif
