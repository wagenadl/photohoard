// ControlSliders.h

#ifndef CONTROLSLIDERS_H

#define CONTROLSLIDERS_H

#include <QScrollArea>
#include <QMap>
#include "Adjustments.h"

class ControlSliders: public QScrollArea {
  /* ControlSliders - This is both the full contents of the main "sliders"
     page [which may be renamed "base" at some point] and part of the contents
     of the "layers" page.
   */
  Q_OBJECT;
public:
  ControlSliders(bool ro, QWidget *parent=0);
  ControlSliders(QWidget *parent=0);
  virtual ~ControlSliders();
  class ControlGroup *group(QString groupname) const; // groupname must exist
  class GentleJog *jog(QString slidername) const; // slidername must exist
  Adjustments const &getAll() const;
  virtual QSize sizeHint() const override;
  static class Actions const &actions();
  static double sliderValue(Adjustments const &vv, QString slider);
  void enterEvent(QEnterEvent *) override;
  void leaveEvent(QEvent *) override;
signals:
  void valuesChanged();
  /* VALUESCHANGED - Emitted when the user changes a slider
     Use GETALL() to get the new values.
  */
  void enterLeave(bool);
public slots:
  void setAll(Adjustments const &vv); // does not signal VALUECHANGED
  void setLayer(int); // enable or disable geometry group
protected slots:
  void goNext(QString);
  void goPrevious(QString);
private slots:
  void sliderChange(QString slidername);
private:
  Adjustments adj;
  QMap<QString, ControlGroup *> groups;
  QMap<QString, GentleJog *> jogs;
  QMap<QString, QString> next;
  QMap<QString, QString> previous;
};


#endif
