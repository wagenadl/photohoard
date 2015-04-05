// TagDialog.h

#ifndef TAGDIALOG_H

#define TAGDIALOG_H

#include <QDialog>
#include "PhotoDB.h"
#include "Tags.h"

class TagDialog: public QDialog {
  Q_OBJECT;
public:
  enum ShowWhat {
    AllDefined,
    UsedInFilter,
    UsedInLibrary,
    UsedInSelectedVersions,
  };
public:
  TagDialog(PhotoDB const &db, bool readOnly=false);
  virtual ~TagDialog();
  void setShown(TagDialog::ShowWhat);
  QList<int> selectedTags();
  void reset();
  int terminalTag() const; // selected tag in rightmost list, if any
signals:
  void apply(int tag);
  void unapply(int tag);
private slots:
  void updateSelection(int level);
  void confirmEdit(int level);
  void clickApply();
  void clickAdd();
  void clickRemove();
  void clickDelete();
  void clickClose();
  void edited();
private:
  void rebuild(int level);
  int selectedTag(int level) const;
  void updateRemoveButton();
  void updateApplyButton();
  void updateAddButton();
  void updateDeleteButton();
  void updateButtons();
protected:
  PhotoDB db;
  Tags tags;
  class QGridLayout *lay;
  QList<class TagListWidget *> lists;
  QList<class QLineEdit *> editors;
  class QSignalMapper *selectionMapper;
  class QSignalMapper *editMapper;
  class QPushButton *addButton;
  class QPushButton *deleteButton;
  class QPushButton *removeButton;
  class QPushButton *applyButton;
  ShowWhat sw;
  int level;
  bool readonly;
};

#endif
