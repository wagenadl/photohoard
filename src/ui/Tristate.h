// Tristate.h

#ifndef TRISTATE_H

#define TRISTATE_H

#include <QWidget>

class Tristate: public QWidget {
  Q_OBJECT;
public:
  enum State {
    Off,
    Undef,
    On,
  };
public:
  Tristate(QWidget *parent=0);
  Tristate(QString text, QWidget *parent=0);
  virtual ~Tristate();
  State state() const { return s; }
  QString text() const { return txt; }
  virtual QSize sizeHint() const override;
  virtual QSize minimumSizeHint() const override;
signals:
  void stateChanged(State);
  /* STATECHANGED - Emitted whenever the state changes
     Whether by direct user action or programmatically.
  */
  void clicked();
  /* CLICKED - Emitted when the user clicks on the widget
     The state will already have been updated.
  */
  void toggled(bool);
  /* TOGGLED - Emitted when the state becomes definitive
     Emitted whenever the state becomes On or Off, but not when it becomes
     Undef. Not emitted in response to SETSTATE.
  */
public slots:
  void setState(State);
  void setText(QString);
protected:
  virtual void mousePressEvent(QMouseEvent *) override;
  virtual void paintEvent(QPaintEvent *) override;
private:
  State s;
  QString txt;
};

#endif
