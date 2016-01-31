// AllControls.h

#ifndef ALLCONTROLS_H

#define ALLCONTROLS_H

#include <QScrollArea>
#include <QMap>
#include "Adjustments.h"

class AllControls: public QScrollArea {
  Q_OBJECT;
public:
  AllControls(QWidget *parent=0);
  virtual ~AllControls();
  class ControlGroup *group(QString groupname) const; // groupname must exist
  class GentleJog *jog(QString slidername) const; // slidername must exist
  Adjustments const &getAll() const;
  virtual QSize sizeHint() const override;
  static class Actions const &actions();
signals:
  void valueChanged(QString adjustmentname, double value);
  /* VALUECHANGED - Emitted when the user changes the value.
     Note that the key is an adjustment name (see AdjustmentDefs.h) rather
     than a slider name. Mostly these are 1:1, but not quite.
  */
public slots:
  void setAll(Adjustments const &vv); // does not signal VALUECHANGED
protected slots:
  void goNext(QString);
  void goPrevious(QString);
private slots:
  void sliderChange(QString slidername);
  void setAndEmit(QString k, double v);
private:
  Adjustments adj;
  QMap<QString, ControlGroup *> groups;
  QMap<QString, GentleJog *> jogs;
  QMap<QString, QString> next;
  QMap<QString, QString> previous;
};

#endif
