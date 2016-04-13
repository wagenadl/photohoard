// MultiDragDialog.h

#ifndef MULTIDRAGDIALOG_H

#define MULTIDRAGDIALOG_H

#include <QWidget>
#include <QSet>

class MultiDragDialog: public QWidget {
  Q_OBJECT;
public:
  MultiDragDialog(class SessionDB *db, QSet<quint64> const &sel, quint64 id);
  virtual ~MultiDragDialog();
private:
  SessionDB *db;
  QSet<quint64> sel;
  quint64 id;
  class Ui_MultiDragDialog *ui;
};

#endif
