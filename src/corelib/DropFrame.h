// DropFrame.h

#ifndef DROPFRAME_H

#define DROPFRAME_H

#include <QLabel>
#include <QUrl>

class DropFrame: public QLabel {
  Q_OBJECT;
public:
  DropFrame(QWidget *parent=0);
  virtual ~DropFrame() { }
signals:
  void dropped(QList<QUrl>, Qt::DropAction);
protected:
  virtual void dragEnterEvent(QDragEnterEvent *) override;
  virtual void dragLeaveEvent(QDragLeaveEvent *) override;
  virtual void dragMoveEvent(QDragMoveEvent *) override;
  virtual void dropEvent(QDropEvent *) override;
private:
  void markFrame(bool in);
};

#endif
