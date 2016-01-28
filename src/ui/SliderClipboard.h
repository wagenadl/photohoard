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
  SliderClipboard(QWidget *parent=0);
  virtual ~SliderClipboard();
  Adjustments values() const; // ignores mask
  QSet<QString> mask() const;
  void get(Adjustments *dest) const;
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
protected slots:
  void goNext(QString);
  void goPrevious(QString);
  void groupStateChange(QString);
private:
  void autoResize();
private:
  QMap<QString, class QCheckBox *> groupControl;
  QMap<QString, QFrame *> groupFrame;
  QMap<QString, QSet<QString> > groupContents;
  QMap<QString, QString> reverseMap;
  QMap<QString, QCheckBox *> jogs;
  QMap<QString, QString> nextThing;
  QMap<QString, QString> previousThing;
  Adjustments val;
};

#endif
