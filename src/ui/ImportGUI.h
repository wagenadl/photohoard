// ImportGUI.h

#ifndef IMPORTGUI_H

#define IMPORTGUI_H

#include <QObject>
#include <QList>
#include <QUrl>

class ImportGUI: public QObject {
  Q_OBJECT;
public:
  ImportGUI(class SessionDB *db,
	    class Scanner *scanner,
	    QList<QUrl> const &sources,
	    QObject *parent=0);
  virtual ~ImportGUI();
  void showAndGo();
private slots:
  void finishUpCompletedJob(QString errmsg); 
  void cleanUpCanceledJob();
  void dlgAcceptExternal();
  void dlgAcceptLocal();
  void dlgAcceptOtherUser();
  void dlgCancel();
private:
  void showAndGoExternal();
  void showAndGoLocal();
  void showAndGoOtherUser();
private:
  class ImportJob *job;
  class ImportExternalDialog *extDlg;
  class ImportOtherUserDialog *otherUserDlg;
  class ImportLocalDialog *localDlg;
  class QProgressDialog *progressDlg; // our responsibility, but not our child
};

#endif
