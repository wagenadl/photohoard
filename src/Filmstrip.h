// Filmstrip.h

#ifndef FILMSTRIP_H

#define FILMSTRIP_H

class Filmstrip: public QGraphicsObject {
  Q_OBJECT;
public:
  enum class Arrangement {
    Horizontal,
    Vertical,
    Grid,
    CompactGrid
  };
  enum class TimeScale {
    Decade,
    Year,
    Month,
    Day,
    Hour,
    DecaMinute,
  };
public:
  Filmstrip(PhotoDB const &db, QGraphicsItem *parent);
  QDateTime startDateTime() const;
  QDateTime endDateTime() const;
  TimeScale timeScale() const;
  QRectF ;// ...
  void paintEvent; //...
  Filmstrip *findDescendentByDate(QDateTime d);
  /* Returns the strip that would directly host a picture with given date.
     That might be us or a (grand)child. It can also be NULL if the date
     is outside our range. */
  Filmstrip *findByVersion(quint64 vsn);
  /* Returns the strip that directly hosts the given version.  That
     might be us or a (grand)child. It can also be NULL. */
signals:
  void needImage(quint64, QSize);
public slots:
  void updateImage(quint64, QSize, QImage);
  void setTimeRange(QDate d0, QDate d1);
  void setTimeRange(QDate d, QTime t0, QTime t1);
  void setTimeRange(QDateTime t0, TimeScale scl);
  void setArrangement(Arregement arr);
  void setTileSize(int pix);
  void setRowWidth(int pix);
  void expand();
  void collapse();
  void expandAll();
  void collapseAll();
  /* I have no idea yet how to deal with change in photo counts. Under
     what circumstances should I change whether I split into
     children? */
private:
  PhotoDB db;
  bool kaput; // i.e., must relayout
  QList<QGraphicsItem *> orderedChildren; // ordered by date
  QSet<Filmstrip *> subStrips; // some of our children are strips
  QSet<Slide *> slides; // other children are image slides
  Header *header;
  
};

#endif
