// CollapseButton.h

#ifndef COLLAPSEBUTTON_H

#define COLLAPSEBUTTON_H

#include <QLabel>

/* This class inspired by
   https://stackoverflow.com/questions/32476006/how-to-make-an-expandable-collapsable-section-widget-in-qt
 */

class CollapseButton: public QLabel {
public:
  CollapseButton(QWidget *parent);
  CollapseButton(QString text, QWidget *parent);
  ~CollapseButton();
  void setText(const QString &text);
  QString text() const;
  void setTarget(QWidget *target);
  void hideTarget();
  void showTarget();
  void mousePressEvent(QMouseEvent *) override;
private:
  bool _shown;
  QString _text;
  QWidget *_target;
};

#endif
