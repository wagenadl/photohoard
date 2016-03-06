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
  void setMask(QSet<QString>);
  void enableAll(bool on=true);
  void disableAll(bool off=true);
  void enableGroup(QString name, bool on=true);
  void disableGroup(QString name, bool off=true);
  void enable(QString name, bool on=true);
  void disable(QString name, bool off=true);
  void copy();
  void apply();
protected slots:
  void goNext(QString);
  void goPrevious(QString);
  void groupStateChange(QString);
private:
  void autoResize();
private:
  bool valok;
  PhotoDB *db;
  AutoCache *ac;
  QMap<QString, class QCheckBox *> groupControl;
  QMap<QString, QFrame *> groupFrame;
  QMap<QString, QSet<QString> > groupContents;
  QMap<QString, QString> reverseMap;
  QMap<QString, QCheckBox *> jogs;
  QMap<QString, QString> nextThing;
  QMap<QString, QString> previousThing;
  Adjustments val;
  QWidget *applyButton;
};

#endif
