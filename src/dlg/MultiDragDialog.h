// MultiDragDialog.h

#ifndef MULTIDRAGDIALOG_H

#define MULTIDRAGDIALOG_H

#include <QDialog>
#include <QSet>

class MultiDragDialog: public QDialog {
  Q_OBJECT;
public:
  MultiDragDialog(class SessionDB *db, class Exporter *,
                  QSet<quint64> const &set, QWidget *parent=0);
  virtual ~MultiDragDialog();
  static bool shouldNotShow() { return dontshow; }
private slots:
  void dropParentFolder(QString);
  void dropSibling(QString);
  void setDontShowAgain(bool);
  void openExportSettings();
private:
  void copyInto(QString);
private:
  SessionDB *db;
  class Exporter *exporter;
  QSet<quint64> set;
  class Ui_MultiDragDialog *ui;
private:
  static bool dontshow;
};

#endif
