// GentleJog.h

#ifndef GENTLEJOG_H

#define GENTLEJOG_H

#include <QLabel>

class GentleJog: public QFrame {
  Q_OBJECT;
public:
  GentleJog(QWidget *parent=0);
  GentleJog(QString lbl, QWidget *parent=0);
  virtual ~GentleJog();
public:
  virtual QSize minimumSizeHint() const override;
  virtual QSize sizeHint() const  override;
public slots:
  void setDefault(double);
  void setMinimum(double);
  void setMaximum(double);
  void setRange(double, double);
  void setDecimals(int);
  void setValue(double); // does cause signal to be emitted
  void setValueQuietly(double); // does not cause signal to be emitted
  void setMaxDelta(double);
  void setPageStep(double);
  void setSingleStep(double);
  void setMicroStep(double);
  void setSteps(double single, double page=-1, double micro=-1);
  void setLabel(QString);
public:
  double defaultValue() const { return dflt_; }
  double minimum() const { return min_; }
  double maximum() const { return max_; }
  int decimals() const { return dec; }
  double value() const { return val; }
  double maxDelta() const { return maxdelta; }
  double singleStep() const { return singlestep; }
  double pageStep() const { return pagestep; }
  double microStep() const { return microstep; }
  QString label() const { return lbl; }
signals:
  void valueChanged(double);
  /* Emitted when the user changes the value and also when changed
     programmatically except when using setValueQuietly. */
protected:
  virtual void paintEvent(QPaintEvent *) override;
  virtual void mousePressEvent(QMouseEvent *) override;
  virtual void mouseReleaseEvent(QMouseEvent *) override;
  virtual void mouseMoveEvent(QMouseEvent *) override;
  virtual void mouseDoubleClickEvent(QMouseEvent *) override;
  virtual void keyPressEvent(QKeyEvent *) override;
  virtual void keyReleaseEvent(QKeyEvent *) override;
  virtual void enterEvent(QEvent *) override;
  virtual void leaveEvent(QEvent *) override;
  virtual void resizeEvent(QResizeEvent *) override;
  virtual void focusInEvent(QFocusEvent *) override;
  virtual void focusOutEvent(QFocusEvent *) override;
  virtual void wheelEvent(QWheelEvent *) override;
private slots:
  void timeout();
private:
  QRect valueRect() const;
  QRect labelRect() const;
  void renderLabel(class QStylePainter *);
  void renderValue(class QStylePainter *);
  void renderJog(class QStylePainter *);
  bool clamp();
  QString valueText() const;
  void resetJog();
  int deltaToX(double d);
  double xToDelta(int x);
  void startJog(int x, Qt::KeyboardModifiers m);
  void endJog(int x);
  void continueJog(int x);
  double stepFor(Qt::KeyboardModifiers m);
  void setValueVisually(double);
  static double mapfwd(double);
  static double maprev(double);
private:
  double dflt_;
  double min_, max_;
  double microstep, singlestep, pagestep, maxdelta;
  double val;
  int dec;
  int jogminx, jogmaxx, jogx, jogr;
  double jogdelta;
  int jogdx0;
  double jogv0;
  bool jogging, jogshown;
  QString lbl;
  class QTimer *timer;
};

#endif
