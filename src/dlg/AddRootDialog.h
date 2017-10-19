// AddRootDialog.h

#ifndef ADDROOTDIALOG_H

#define ADDROOTDIALOG_H

#include <QDialog>
#include <QModelIndex>

class AddRootDialog: public QDialog {
  Q_OBJECT;
public:
  AddRootDialog(class PhotoDB *db, QWidget *parent=0);
  virtual ~AddRootDialog();
  virtual int exec();
  QString path() const;
  QString defaultCollection() const;
  QStringList exclusions() const;
  bool validate(bool interactive=false) const;
  /* Validate returns true if (1) a folder has been selected, (2) that
     folder exists, and (3) a collection has been selected. The interactive
     flag determines whether warning boxes are displayed to the user if
     validation fails.
   */
protected:
  void keyPressEvent(QKeyEvent *) override;
protected slots:
  void browse();
  void addExclusion();
  void removeExclusion();
  void editExclusion(QModelIndex);
private:
  void prepCollections();
  QString reasonableStartingPoint();
private:
  class Ui_addRootDialog *ui;
  class PhotoDB *db;
};

#endif
