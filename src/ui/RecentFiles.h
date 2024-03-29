// RecentFiles.h

#ifndef RECENTFILES_H

#define RECENTFILES_H

#include <QString>
#include <QMenu>

class RecentFiles: public QMenu {
  Q_OBJECT;
public:
  static const int MAXFILES = 9;
  RecentFiles(QString varname="recentdbs", QWidget *parent=0,
              QString mydbfn="");
  void mark(QString fn);
  QStringList list() const;
  void showEvent(QShowEvent *) override;
signals:
  void selected(QString);
private:
  void updateItems();
private:
  QString varname;
  QAction *actions[MAXFILES];
  QString mydbfn;
};

#endif
