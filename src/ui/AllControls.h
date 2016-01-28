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
  class ControlGroup *group(QString groupname) const;
  class GentleJog *jog(QString slidername) const;
  Adjustments const &getAll() const;
  virtual QSize sizeHint() const override;
  static class Actions const &actions();
signals:
  void valueChanged(QString adjustmentname, double value);
  /* Emitted when the user changes the value and also when changed
     programmatically except for setQuietly. */
public slots:
  void setAll(Adjustments const &vv); // does not signal
protected slots:
  void goNext(QString);
  void goPrevious(QString);
private slots:
  void sliderChange(QString slidername);
private:
  double sliderValue(QString slidername) const;
private:
  Adjustments adj;
  QMap<QString, ControlGroup *> groups;
  QMap<QString, GentleJog *> jogs;
  QMap<QString, QString> next;
  QMap<QString, QString> previous;
};

#endif
