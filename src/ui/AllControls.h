// AllControls.h

#ifndef ALLCONTROLS_H

#define ALLCONTROLS_H

#include <QFrame>
#include <QMap>

class AllControls: public QFrame {
  Q_OBJECT;
public:
  AllControls(QWidget *parent=0);
  virtual ~AllControls();
  class ControlGroup *group(QString name) const;
  class GentleJog *jog(QString name) const;
  double get(QString name) const; // nan if nonexistent
  bool contains(QString name) const { return jogs.contains(name); }
signals:
  void valueChanged(QString name, double value);
  /* Emitted when the user changes the value and also when changed
     programmatically except for setQuietly. */
public slots:
  bool set(QString name, double value); // true if OK; does signal
  bool setQuietly(QString name, double value); // true if OK; does not signal
  void setQuietly(class Sliders const &vv); // does not signal
private slots:
  void valueChange(QString name);
private:
  QMap<QString, ControlGroup *> groups;
  QMap<QString, GentleJog *> jogs;
};

#endif
