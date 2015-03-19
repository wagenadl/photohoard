// AppliedTagEditor.h

#ifndef APPLIEDTAGEDITOR_H

#define APPLIEDTAGEDITOR_H

#include <QFrame>
#include "Tags.h"

class AppliedTagEditor: public QFrame {
  Q_OBJECT;
public:
  AppliedTagEditor(PhotoDB const &db, QWidget *parent=0);
  virtual ~AppliedTagEditor() { }
  QString text() const;
  QSize sizeHint() const override;
  QSize minimumSizeHint() const override;
public slots:
  void reset();
  void setText(QString);
signals:
  void returnPressed();
protected:
  virtual void mousePressEvent(QMouseEvent *) override;
  virtual void keyPressEvent(QKeyEvent *) override;
  virtual void paintEvent(QPaintEvent *) override;
  virtual void focusInEvent(QFocusEvent *) override;
  virtual void focusOutEvent(QFocusEvent *) override;
private:
  void deleteSelection();
  int selectionStart();
  int selectionEnd();
private:
  Tags tags;
  QString txt;
  int cursorpos;
  int selend; // -1 for no selection
};

#endif
