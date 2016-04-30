// MultiDragDialog.h

#ifndef MULTIDRAGDIALOG_H

#define MULTIDRAGDIALOG_H

#include <QWidget>
#include <QSet>

class MultiDragDialog: public QWidget {
  Q_OBJECT;
public:
  MultiDragDialog(class SessionDB *db, class Exporter *,
                  QSet<quint64> const &set);
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
