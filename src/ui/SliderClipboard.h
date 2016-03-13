// Sliderclipboard.h

#ifndef SLIDERCLIPBOARD_H
#define SLIDERCLIPBOARD_H

#include <QScrollArea>
#include <QMap>
#include <QSet>
#include "Adjustments.h"

class SliderClipboard: public QScrollArea {
  Q_OBJECT;
public:
  SliderClipboard(class PhotoDB *db, class AutoCache *ac, QWidget *parent=0);
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
private:
  bool valok;
  PhotoDB *db;
  AutoCache *ac;
  QMap<QString, class Tristate *> groupControl;
  QMap<QString, QFrame *> groupFrame;
  QMap<QString, QSet<QString> > groupContents;
  QMap<QString, QString> containingGroup;
  QMap<QString, class QCheckBox *> jogs;
  QMap<QString, QString> nextThing;
  QMap<QString, QString> previousThing;
  Adjustments val;
  QWidget *applyButton;
};

#endif
