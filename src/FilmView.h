// FilmView.h

#ifndef FILMVIEW_H

#define FILMVIEW_H

#include <QGraphicsView>

class FilmView: public QGraphicsView {
  Q_OBJECT;
public:
  FilmView(class PhotoDB const &db, QWidget *parent=0);
  virtual ~FilmView();
signals:
  void needImage(quint64, QSize);
  void pressed(quint64);
  void clicked(quint64);
  void doubleClicked(quint64);
public slots:
  void updateImage(quint64, QSize, QImage);
protected:
  virtual void resizeEvent(QResizeEvent *) override;
protected slots:
  void stripResized();
protected:
  void setScrollbarPolicies();
private:
  class QGraphicsScene *scene;
  class Datestrip *strip;
};

#endif
