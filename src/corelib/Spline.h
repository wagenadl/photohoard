// Spline.h

#ifndef SPLINE_H

#define SPLINE_H

#include <QPolygon>
#include <QList>

class Spline {
public:
  Spline(QPolygonF const &orig=QPolygonF(),
         int downsample=1, bool closed=true);
  // By default, the distance between adjacent interpolated points is
  // about 1 pixel. Setting DOWNSAMPLE to a value greater than one
  // increases that distance accordingly.
  
  // If CLOSED is false, no spline is drawn from the last point back
  // to the first point of the original polygon, so the result,
  // strictly speaking, is a polyline rather than a polygon.
public:
  QPolygonF points;
  QList<int> origidx; // index in POINTS of original polygon
private:
  struct Params {
    double p[4];
  };
private:
  static Params calcOne(double before, double start,
                        double end, double after);
  static QPolygonF buildOne(Params x, Params y, int n);
  static int divisions(QPointF const &start, QPointF const &end);
};

#endif
