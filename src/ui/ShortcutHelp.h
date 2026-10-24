// ShortcutHelp.h

#ifndef SHORTCUTHELP_H

#define SHORTCUTHELP_H

#include <QDialog>

class ShortcutHelp: public QDialog {
public:
  ShortcutHelp(QWidget *parent=0);
  ~ShortcutHelp();
  void addSection(QString label, class Actions const &contents);
  QSize sizeHint() const override;
protected:
  virtual void resizeEvent(QResizeEvent *);
private:
  class SHPrivate *d;
};

#endif
