// MultiDragDialog.h

#ifndef MULTIDRAGDIALOG_H

#define MULTIDRAGDIALOG_H

#include <QWidget>
#include <QSet>

class MultiDragDialog: public QWidget {
  Q_OBJECT;
public:
  MultiDragDialog(class SessionDB *db, QSet<quint64> const &set);
  virtual ~MultiDragDialog();
  static bool shouldNotShow() { return dontshow; }
private slots:
  void dropParentFolder(QString);
  void dropSibling(QString);
  void setDontShowAgain(bool);
  void exportComplete(QString, int, int);
  void exportProgress(int, int);
private:
  void copyInto(QString);
private:
  SessionDB *db;
  QSet<quint64> set;
  class Ui_MultiDragDialog *ui;
private:
  class Exporter *exporter;
  class QProgressDialog *progress;
private:
  static bool dontshow;
};

#endif
