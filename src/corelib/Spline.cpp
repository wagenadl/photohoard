// Spline.cpp

#include "Spline.h"
#include "PDebug.h"

Spline::Params Spline::calcOne(double before, double start,
                               double end, double after) {
  static double mat[4][4]{ { -0.5,  1.5, -1.5,  0.5 },
                           {  1.0, -2.5,  2.0, -0.5 },
                           { -0.5,  0.0,  0.5,  0.0 },
                           {  0.0,  1.0,  0.0,  0.0 } };
  double in[4]{ before, start, end, after };
  Params p;
  for (int k=0; k<4; k++) {
    double v = 0;
    for (int m=0; m<4; m++)
      v += mat[k][m]*in[m];
    p.p[k] = v;
  }
  return p;
}

QPolygonF Spline::buildOne(Spline::Params px, Spline::Params py, int N) {
  QPolygonF pp(N);
  for (int n=0; n<N; n++) {
    double t = n*1.0 / N;
    double t2 = t*t;
    double t3 = t*t*t;
    double x = t3*px.p[0] + t2*px.p[1] + t*px.p[2] + px.p[3];
    double y = t3*py.p[0] + t2*py.p[1] + t*py.p[2] + py.p[3];
    pp[n] = QPointF(x, y);
  }
  return pp;
}

int Spline::divisions(QPointF const &start, QPointF const &end) {
  return (start-end).manhattanLength();
}

Spline::Spline(QPolygonF const &p0,
               int downsample,
               bool periodic) {

  int N = p0.size();
  if (N==0)
    return;
  
  int M = periodic ? N : N-1;
  
  for (int n=0; n<M; n++) {
    int k_1 = n-1;
    if (k_1<0)
      k_1 = periodic ? (N-1) : 0;
    int k0 = n;
    int k1 = n + 1;
    if (k1>=N)
      k1 -= N; // this can only happen for periodic
    int k2 = n + 2;
    if (k2>=N)
      k2 = periodic ? (k2-N) : N-1;

    Params px = calcOne(p0[k_1].x(), p0[k0].x(), p0[k1].x(), p0[k2].x());
    Params py = calcOne(p0[k_1].y(), p0[k0].y(), p0[k1].y(), p0[k2].y());

    int q = divisions(p0[k0], p0[k1]) / downsample;
    if (q<1)
      q = 1;

    origidx << points.size();
    points << buildOne(px, py, q);
  }
  if (!periodic) {
    origidx << points.size();
    points << p0[N-1];
  }
}
