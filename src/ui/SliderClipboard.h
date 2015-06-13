// Sliderclipboard.h

#ifndef SLIDERCLIPBOARD_H
#define SLIDERCLIPBOARD_H

#include <QScrollArea>
#include <QMap>
#include <QSet>
#include "Sliders.h"

class SliderClipboard: public QScrollArea {
  Q_OBJECT;
public:
  SliderClipboard(QWidget *parent=0);
  virtual ~SliderClipboard();
  Sliders values() const; // ignores mask
  QSet<QString> mask() const;
  void get(Sliders *dest) const;
public slots:
  void set(class Sliders const &vv);
  void setAll(class Sliders const &vv); // ignores mask
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
  Sliders val;
};

#endif
