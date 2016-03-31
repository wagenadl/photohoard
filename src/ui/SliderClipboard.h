// Sliderclipboard.h

#ifndef SLIDERCLIPBOARD_H
#define SLIDERCLIPBOARD_H

#include <QDialog>
#include <QMap>
#include <QSet>
#include "Adjustments.h"

class SliderClipboard: public QDialog {
  Q_OBJECT;
public:
  SliderClipboard(class SessionDB *db, class AutoCache *ac, QWidget *parent=0);
  virtual ~SliderClipboard();
  Adjustments values() const; // ignores mask
  QSet<QString> mask() const;
  void get(Adjustments *dest) const;
signals:
  void modified(quint64 version);
public slots:
  void set(class Adjustments const &vv);
  void setAll(class Adjustments const &vv); // ignores mask
  void enableAll(bool on=true);
  void disableAll(bool off=true);
  void copy();
  void apply();
protected slots:
  void goNext(QString);
  void goPrevious(QString);
  void groupStateChange(QString);
  void sliderStateChange(QString);
private:
  void autoResize();
  static int decimals(QString sli);
private:
  bool valok;
  SessionDB *db;
  AutoCache *ac;
  QMap<QString, class Tristate *> groupControl;
  QMap<QString, class QFrame *> groupFrame;
  QMap<QString, QSet<QString> > groupContents;
  QMap<QString, QString> containingGroup;
  QMap<QString, class QCheckBox *> jogs;
  QMap<QString, class QLabel *> labels;
  QMap<QString, QString> nextThing;
  QMap<QString, QString> previousThing;
  class QScrollArea *sa;
  Adjustments val;
  QWidget *applyButton;
};

#endif
