// Spline.h

#ifndef SPLINE_H

#define SPLINE_H

#include <QPolygon>

class Spline: public QPolygon {
public:
  static QPolygonF catmullRom(QPolygonF const &, int downsample=1,
                              int *idxout=0);
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
