// ShortcutHelp.h

#ifndef SHORTCUTHELP_H

#define SHORTCUTHELP_H

#include <QDialog>

class ShortcutHelp {
public:
  ShortcutHelp();
  ~ShortcutHelp();
  void addSection(QString label, class Actions const &contents);
  QString html(QFont const &) const;
private:
  class SHPrivate *d;
};


class ShortcutHelpDialog: public QDialog {
public:
  ShortcutHelpDialog(ShortcutHelp *data, QWidget *parent);
  ~ShortcutHelpDialog();
  QSize sizeHint() const override;
protected:
  void focusOutEvent(QFocusEvent *) override;
private:
  class QVBoxLayout *layout;
  class SHEditor *editor;
};

#endif
