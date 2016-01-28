// AllControls.h

#ifndef ALLCONTROLS_H

#define ALLCONTROLS_H

#include <QScrollArea>
#include <QMap>

class AllControls: public QScrollArea {
  Q_OBJECT;
public:
  AllControls(QWidget *parent=0);
  virtual ~AllControls();
  class ControlGroup *group(QString groupname) const;
  class GentleJog *jog(QString slidername) const;
  double get(QString adjustmentname) const; // nan if nonexistent
  virtual QSize sizeHint() const override;
  static class Actions const &actions();
signals:
  void valueChanged(QString adjustmentname, double value);
  /* Emitted when the user changes the value and also when changed
     programmatically except for setQuietly. */
public slots:
  bool set(QString adjustmentname, double value); // true if OK; does signal
  bool setQuietly(QString adjustmentname, double value); // true if OK; does not signal
  void setQuietly(class Adjustments const &vv); // does not signal
protected slots:
  void goNext(QString);
  void goPrevious(QString);
private slots:
  void sliderChange(QString slidername);
private:
  QMap<QString, ControlGroup *> groups;
  QMap<QString, GentleJog *> jogs;
  QMap<QString, QString> next;
  QMap<QString, QString> previous;
};

#endif
