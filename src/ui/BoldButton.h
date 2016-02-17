// BoldButton.h

#ifndef BOLDBUTTON_H

#define BOLDBUTTON_H

#include <QPushButton>

class BoldButton: public QPushButton {
  Q_OBJECT;
public:
  BoldButton(QString txt, QWidget *parent=0);
  virtual ~BoldButton();
  virtual QSize sizeHint() const;
private slots:
  void reflect();
private:
  mutable QSize ss;
};

#endif
