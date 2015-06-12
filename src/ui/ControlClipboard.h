// Controlclipboard.h

#ifndef CONTROLCLIPBOARD_H

#define CONTROLCLIPBOARD_H

#include <QScrollArea>
#include <QMap>
#include "Sliders.h"

class Controlclipboard: public QScrollArea {
  Q_OBJECT;
public:
  ControlClipboard(QWidget *parent=0);
  virtual ~ControlClipboard();
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
  QMap<QString, QCheckBox *> groupControl;
  QMap<QString, QFrame *> groupFrame;
  QMap<QString, QSet<QString> > groupContents;
  QMap<QString, QString> reverseMap;
  QMap<QString, QCheckBox *> jogs;
  QMap<QString, QString> nextThing;
  QMap<QString, QString> previousThing;
  Sliders val;
};
