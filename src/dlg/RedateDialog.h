// RedateDialog.h

#ifndef REDATEDIALOG_H

#define REDATEDIALOG_H

#include <QDialog>
#include <QMap>
#include <QDateTime>

class RedateDialog: public QDialog {
  Q_OBJECT;
public:
  RedateDialog(class PhotoDB *db,
               QList<quint64> versions, quint64 keyvsn,
               bool isimport,
               QWidget *parent=0);
  virtual ~RedateDialog();
public slots:
  void apply();
signals:
  void applied();
private:
  class Ui_RedateDialog *ui;
  class PhotoDB *db;
  QMap<quint64, QDateTime> olddtt;
  QDateTime dtkey;
};

#endif
